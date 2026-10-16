#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "motors.h"
#include "display.h"

// Thông tin 3 Map (Thay đổi 3 chỉ số đầu khi tuning)
MapProfile profiles[3] = {
    {"MAP 1", 0.024f, 0.165f, 195, 0.011f, true, true},
    {"MAP 2", 0.031f, 0.230f, 175, 0.015f, false, true},
    {"MAP 3", 0.022f, 0.155f, 210, 0.012f, true, true}};
int selectedMap = 0;

// Biến trạng thái
RobotState state = STATE_IDLE;
float lastError = 0;
float filteredD = 0;
int straightCount = 0;
unsigned long lastControlTick = 0;
unsigned long lastOledTick = 0;
unsigned long turnStartTime = 0;

// Hàm reset bộ nhớ điều khiển để tránh giật xe khi chuyển trạng thái
void resetControllerState()
{
    lastError = currentError;
    filteredD = 0;
    straightCount = 0;
}

// Tính toán PD và điều khiển tốc độ thích nghi an toàn
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

    // 1. Khi đi thẳng (lệch rất ít và vạch không dịch chuyển ngang nhanh)
    if (absErr < 800 && abs(filteredD) < 150)
    {
        straightCount++;
        if (straightCount > 20)
        {
            // Tăng tốc mượt (Ramp-up): mỗi ms tăng +1 PWM, tối đa +35 để chống trượt bánh
            int boost = min(straightCount - 20, 35);
            dynamicBase = min(cfg.maxBaseSpeed + boost, 250);
        }
        else
            dynamicBase = cfg.maxBaseSpeed;
    }
    // 2. Khi vào cua: Phanh bậc 2 theo vị trí + Phanh sớm theo tốc độ lệch vạch (filteredD)
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
    WiFi.mode(WIFI_OFF);
    btStop();

    setupSensors();
    setupMotors();
    setupDisplay();

    pinMode(PIN_CALIB, INPUT_PULLUP);
    pinMode(PIN_START, INPUT_PULLUP);
}

void loop()
{

    // TRẠNG THÁI 1: CHỜ XUẤT PHÁT
    // ===========================
    if (state == STATE_IDLE)
    {
        setMotors(0, 0);
        analyzeLine();

        if (millis() - lastOledTick >= 50)
        {
            lastOledTick = millis();
            updateOledDashboard(selectedMap, profiles, currentError, leftBlackCount, rightBlackCount);
        }
        // Nút CALIB: Nhấn nhả đổi Map, nhấn giữ > 600ms để Calib
        if (digitalRead(PIN_CALIB) == LOW)
        {
            unsigned long pressTime = millis();
            while (digitalRead(PIN_CALIB) == LOW)
            {
                if (millis() - pressTime > 600)
                {
                    runAutoCalibration();
                    while (digitalRead(PIN_CALIB) == LOW)
                        ;
                    return;
                }
            }
            selectedMap = (selectedMap + 1) % 3;
            delay(150);
        }
        // Nút START: Đếm ngược 1s rồi chạy
        if (digitalRead(PIN_START) == LOW)
        {
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(20, 24);
            display.println("READY...");
            display.display();
            delay(1000);
            display.clearDisplay();
            display.setCursor(15, 24);
            display.println("RUNNING");
            display.display();
            analyzeLine();
            resetControllerState();
            lastControlTick = micros();
            state = STATE_RUNNING;
        }
        return;
    }
    // Nhấn START để dừng khẩn cấp
    if (digitalRead(PIN_START) == LOW)
    {
        setMotors(0, 0);
        delay(500);
        state = STATE_IDLE;
        return;
    }

    // TRẠNG THÁI 2: ĐANG CHẠY
    // =======================
    if (micros() - lastControlTick < 1000)
        return;
    lastControlTick = micros();

    analyzeLine();
    MapProfile &cfg = profiles[selectedMap];

    // 1. Đang thực hiện xoay góc vuông 90 độ
    if (state == STATE_TURN90_L || state == STATE_TURN90_R)
    {
        unsigned long turnElapsed = millis() - turnStartTime;

        if (state == STATE_TURN90_L)
            setMotors(-155, 185);
        else
            setMotors(185, -155);
        // Xoay ít nhất 70ms hoặc tự thoát nếu quá 350ms
        bool centerCaughtLine = (normSensor[6] > 550 || normSensor[7] > 550 || normSensor[8] > 550 || normSensor[9] > 550);
        if ((turnElapsed > 70 && centerCaughtLine) || turnElapsed > 350)
        {
            resetControllerState();
            state = STATE_RUNNING;
        }
        return;
    }

    // 2. Nhận diện góc vuông 90 độ (Khi 1 bên thấy >= 3 mắt đen đậm và bên kia hoàn toàn trắng)
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

    // 3. Xử lý khi mất vạch hoàn toàn (Vạch đứt hoặc Văng cua)
    if (isAllWhite)
    {
        straightCount = 0;
        // Phóng thẳng qua vạch đứt nếu Map đó cho phép
        if (cfg.allowGapJump && abs(lastError) < 900 && abs(filteredD) < 180)
            setMotors(cfg.maxBaseSpeed, cfg.maxBaseSpeed);
        // Mất vạch do văng cua
        else if (lastError > 0)
            setMotors(180, -150);
        else
            setMotors(-150, 180);
        return;
    }

    // 4. Bám line bình thường bằng bộ điều khiển PD
    computePDAndDrive();
}