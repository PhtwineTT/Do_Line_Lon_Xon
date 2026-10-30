#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "motors.h"
#include "display.h"
#include "web_tuning.h"
#define LED_BUILTIN 2

MapProfile profiles[3] = {
    {"MAP 1", 0.024f, 0.165f, 195, 0.011f, true, true},
    {"MAP 2", 0.031f, 0.230f, 225, 0.015f, true, true},
    {"MAP 3", 0.022f, 0.155f, 210, 0.012f, true, true}};
int selectedMap = 0;

RobotState state = STATE_IDLE;
float lastError = 0;
float filteredD = 0;
int straightCount = 0;
unsigned long lastControlTick = 0;
unsigned long lastOledTick = 0;
unsigned long turnStartTime = 0;

void resetControllerState()
{
    lastError = currentError;
    filteredD = 0;
    straightCount = 0;
}

void computePDAndDrive()
{
    MapProfile &cfg = profiles[selectedMap];

    float P = currentError;
    float rawD = currentError - lastError;
    filteredD = 0.7f * filteredD + 0.3f * rawD;

    float pidOutput = (cfg.kp * P) + (cfg.kd * filteredD);
    lastError = currentError;

    float absErr = abs(currentError);
    int dynamicBase;

    if (absErr < 800 && abs(filteredD) < 150)
    {
        straightCount++;
        if (straightCount > 20)
        {
            int boost = min(straightCount - 20, 35);
            dynamicBase = min(cfg.maxBaseSpeed + boost, 250);
        }
        else
            dynamicBase = cfg.maxBaseSpeed;
    }
    else
    {
        straightCount = 0;
        float normErr = absErr / 1000.0f;
        int speedDrop = (int)(cfg.kSlow * (140.0f * (normErr * normErr) + 0.6f * abs(filteredD)));
        dynamicBase = cfg.maxBaseSpeed - speedDrop;
    }

    dynamicBase = constrain(dynamicBase, 105, 250);
    setMotors(dynamicBase + (int)pidOutput, dynamicBase - (int)pidOutput);
}

void setup()
{
    Serial.begin(115200);
    setupWebTuning();

    setupSensors();
    setupMotors();
    setupDisplay();

    // SỬA: Chân 34 không có INPUT_PULLUP nội bộ, dùng INPUT vì đã có trở ngoài R4
    pinMode(PIN_CALIB, INPUT);
    pinMode(PIN_START, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
    if (state == STATE_IDLE)
    {
        handleWebTuning();
        setMotors(0, 0);
        analyzeLine();

        if (millis() - lastOledTick >= 500)
        {
            lastOledTick = millis();
            updateOledDashboard(selectedMap, profiles, currentError, leftBlackCount, rightBlackCount);
        }
        
        // NÚT CALIB: Nhấn nhả nhanh để Đổi Map / Nhấn giữ dài (>600ms) để Calib
        if (digitalRead(PIN_CALIB) == LOW)
        {
            delay(20); // THÊM: Chống nảy phím (Debounce 20ms) thay thế cho tụ C8 đã tháo
            if (digitalRead(PIN_CALIB) == LOW)
            {
                Serial.println(">>> MACH DA NHAN DUOC TIN HIEU BAM NUT!");
                unsigned long pressTime = millis();
                bool isLongPress = false;
                
                // Chờ đến khi nhả nút hoặc quá thời gian giữ
                while (digitalRead(PIN_CALIB) == LOW)
                {
                    if (millis() - pressTime > 600)
                    {
                        isLongPress = true;
                        break;
                    }
                    delay(10);
                }

                if (isLongPress)
                {
                    digitalWrite(LED_BUILTIN, HIGH); 
                    runAutoCalibration();
                    digitalWrite(LED_BUILTIN, LOW);  
                    while (digitalRead(PIN_CALIB) == LOW);
                }
                else
                {
                    selectedMap = (selectedMap + 1) % 3;
                    for (int i = 0; i <= selectedMap; i++) {
                        digitalWrite(LED_BUILTIN, HIGH); delay(180);
                        digitalWrite(LED_BUILTIN, LOW); delay(180);
                    }
                    Serial.print(">>> DA CHUYEN SANG MAP: ");
                    Serial.println(selectedMap + 1);
                }
                delay(200); // Chống dội lặp lệnh khi nhả nút
            }
        }
        
        // NÚT START: Bắt đầu chạy
        if (digitalRead(PIN_START) == LOW)
        {
            delay(20); // Debounce nút START
            if (digitalRead(PIN_START) == LOW)
            {
                Serial.println("READY...");
                // Nháy LED nhanh 3 lần báo hiệu sẵn sàng
                for(int i=0; i<3; i++) {
                    digitalWrite(LED_BUILTIN, HIGH); delay(100);
                    digitalWrite(LED_BUILTIN, LOW); delay(100);
                }
                delay(700); 
                Serial.println("RUNNING!");
                
                analyzeLine();
                resetControllerState();
                lastControlTick = micros();
                state = STATE_RUNNING;
            }
        }
        return;
    }
    
    // Đang chạy: Nhấn lại nút START để dừng
    if (digitalRead(PIN_START) == LOW)
    {
        setMotors(0, 0);
        delay(500);
        state = STATE_IDLE;
        return;
    }

    if (micros() - lastControlTick < 1000)
        return;
    lastControlTick = micros();

    analyzeLine();
    MapProfile &cfg = profiles[selectedMap];

    if (state == STATE_TURN90_L || state == STATE_TURN90_R)
    {
        unsigned long turnElapsed = millis() - turnStartTime;

        if (state == STATE_TURN90_L)
            setMotors(-155, 185);
        else
            setMotors(185, -155);
            
        bool centerCaughtLine = (normSensor[6] > 550 || normSensor[7] > 550 || normSensor[8] > 550 || normSensor[9] > 550);
        if ((turnElapsed > 70 && centerCaughtLine) || turnElapsed > 350)
        {
            resetControllerState();
            state = STATE_RUNNING;
        }
        return;
    }

    if (cfg.allowTurn90)
    {
        if (leftBlackCount >= 3 && rightBlackCount == 0)
        {
            turnStartTime = millis();
            straightCount = 0;
            state = STATE_TURN90_L;
            return;
        }
        if (rightBlackCount >= 3 && leftBlackCount == 0)
        {
            turnStartTime = millis();
            straightCount = 0;
            state = STATE_TURN90_R;
            return;
        }
    }

    if (isAllWhite)
    {
        straightCount = 0;
        if (cfg.allowGapJump && abs(lastError) < 900 && abs(filteredD) < 180)
            setMotors(cfg.maxBaseSpeed, cfg.maxBaseSpeed);
        else if (lastError > 0)
            setMotors(180, -150);
        else
            setMotors(-150, 180);
        return;
    }

    computePDAndDrive();
}