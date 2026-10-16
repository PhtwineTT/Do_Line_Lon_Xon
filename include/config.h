#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Màn hình OLED
#define OLED_SDA -1
#define OLED_SCL -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// IC MUX U24 (Đọc cảm biến)
#define MUX_S0 -1
#define MUX_S1 -1
#define MUX_S2 -1
#define MUX_S3 -1
#define MUX_SIG -1

// Điều khiển động cơ
#define MOT_L_IN1 -1
#define MOT_L_IN2 -1
#define MOT_L_PWM -1
#define CH_PWM_L 0 

#define MOT_R_IN1 -1 
#define MOT_R_IN2 -1
#define MOT_R_PWM -1
#define CH_PWM_R 1

// Nút bấm
#define PIN_CALIB -1
#define PIN_START -1

#define NUM_SENSORS 16

// Đổi thành true nếu khi test trên OLED thấy nền trắng cột cao hơn vạch đen
#define INVERT_SENSORS false

// Cấu trúc thông số của 3 bản đồ
struct MapProfile {
    const char* name;
    float kp;
    float kd;
    int maxBaseSpeed;
    float kSlow;
    bool allowGapJump;
    bool allowTurn90;
};

enum RobotState { STATE_IDLE, STATE_CALIB, STATE_RUNNING, STATE_TURN90_L, STATE_TURN90_R };

#endif