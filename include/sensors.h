#ifndef SENSORS_H
#define SENSORS_H
#include <Arduino.h>
#include "config.h"
extern int muxChannelMap[NUM_SENSORS];
extern int rawSensor[NUM_SENSORS];
extern int minVal[NUM_SENSORS];
extern int maxVal[NUM_SENSORS];
extern int normSensor[NUM_SENSORS];
extern bool isAllWhite;
extern int leftBlackCount;
extern int rightBlackCount;
extern float currentError;
void setupSensors();
void selectMuxChannel(int ch);
void readAllRawSensors();
void resetCalibData();
void readCalibratedSensors();
void analyzeLine();
#endif