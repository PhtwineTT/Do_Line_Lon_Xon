#include "motors.h"

// ESP32 có 16 kênh PWM độc lập
#define CH_L_IN1 0
#define CH_L_IN2 1
#define CH_R_IN1 2
#define CH_R_IN2 3

void setupMotors() {
    // 1. Kích hoạt IC công suất
    pinMode(PIN_MOT_EN, OUTPUT);
    digitalWrite(PIN_MOT_EN, HIGH); 

    // 2. Gắn PWM cho Động cơ TRÁI (Tần số 15kHz, 8-bit)
    ledcSetup(CH_L_IN1, 15000, 8);
    ledcAttachPin(PIN_MOT_L_IN1, CH_L_IN1);
    
    ledcSetup(CH_L_IN2, 15000, 8);
    ledcAttachPin(PIN_MOT_L_IN2, CH_L_IN2);

    // 3. Gắn PWM cho Động cơ PHẢI
    ledcSetup(CH_R_IN1, 15000, 8);
    ledcAttachPin(PIN_MOT_R_IN1, CH_R_IN1);
    
    ledcSetup(CH_R_IN2, 15000, 8);
    ledcAttachPin(PIN_MOT_R_IN2, CH_R_IN2);
}

void setMotors(int speedL, int speedR) {
    speedL = constrain(speedL, -255, 255);
    speedR = constrain(speedR, -255, 255);

    // Bánh trái
    if (speedL >= 0) {
        ledcWrite(CH_L_IN1, speedL);
        ledcWrite(CH_L_IN2, 0);
    } else {
        ledcWrite(CH_L_IN1, 0);
        ledcWrite(CH_L_IN2, -speedL);
    }

    // Bánh phải
    if (speedR >= 0) {
        ledcWrite(CH_R_IN1, speedR);
        ledcWrite(CH_R_IN2, 0);
    } else {
        ledcWrite(CH_R_IN1, 0);
        ledcWrite(CH_R_IN2, -speedR);
    }
}