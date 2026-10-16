#include "motors.h"

// Khởi tạo
void setupMotors() {
    pinMode(MOT_L_IN1, OUTPUT);
    pinMode(MOT_L_IN2, OUTPUT);
    pinMode(MOT_R_IN1, OUTPUT);
    pinMode(MOT_R_IN2, OUTPUT);

    ledcSetup(CH_PWM_L, 15000, 8);
    ledcAttachPin(MOT_L_PWM, CH_PWM_L);

    ledcSetup(CH_PWM_R, 15000, 8);
    ledcAttachPin(MOT_R_PWM, CH_PWM_R);
}

// Thiết lập tốc độ 2 bánh
void setMotors(int speedL, int speedR) {
    speedL = constrain(speedL, -255, 255);
    speedR = constrain(speedR, -255, 255);

    // Bánh trái
    if (speedL >= 0) {
        digitalWrite(MOT_L_IN1, HIGH);
        digitalWrite(MOT_L_IN2, LOW);
        ledcWrite(CH_PWM_L, speedL);
    } else {
        digitalWrite(MOT_L_IN1, LOW);
        digitalWrite(MOT_L_IN2, HIGH);
        ledcWrite(CH_PWM_L, -speedL);
    }

    // Bánh phải
    if (speedR >= 0) {
        digitalWrite(MOT_R_IN1, HIGH);
        digitalWrite(MOT_R_IN2, LOW);
        ledcWrite(CH_PWM_R, speedR);
    } else {
        digitalWrite(MOT_R_IN1, LOW);
        digitalWrite(MOT_R_IN2, HIGH);
        ledcWrite(CH_PWM_R, -speedR);
    }
}