#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "config.h"
#include "sensors.h"

void setupDisplay();
void updateOledDashboard(int selectedMap, MapProfile profiles[], float currentError, int leftCount, int rightCount);
void runAutoCalibration();

#endif