#ifndef MOTORS_H
#define MOTORS_H
#include <Arduino.h>
#include "config.h"
void setupMotors();
void setMotors(int speedL, int speedR);
#endif