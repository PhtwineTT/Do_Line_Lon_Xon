#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// IC MUX U24 (16 Cảm biến)
#define MUX_S0          17
#define MUX_S1          18
#define MUX_S2          19
#define MUX_S3          23
#define MUX_SIG         36
#define NUM_SENSORS     16

// ĐỘNG CƠ TRÁI (IC U27) & ENCODER L
#define PIN_ENC_L1      21
#define PIN_ENC_L2      22
#define PIN_MOT_L_IN1   27
#define PIN_MOT_L_IN2   25

// ĐỘNG CƠ PHẢI (IC U29) & ENCODER R
#define PIN_ENC_R1      32
#define PIN_ENC_R2      33
#define PIN_MOT_R_IN1   26
#define PIN_MOT_R_IN2   13

// Chân Enable chung cho cả 2 IC Motor
#define PIN_MOT_EN      16

// Nút bấm
#define PIN_CALIB       34
#define PIN_START       35

#define INVERT_SENSORS  false

struct MapProfile {
    const char* name;
    float kp;
    float kd;
    int maxBaseSpeed;
    float kSlow;
    bool allowGapJump;
    bool allowTurn90;
};

enum RobotState { 
    STATE_IDLE, 
    STATE_CALIB, 
    STATE_RUNNING, 
    STATE_TURN90_L, 
    STATE_TURN90_R 
};

#endif