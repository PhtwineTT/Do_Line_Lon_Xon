#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "sensors.h"
extern Adafruit_SSD1306 display;
void setupDisplay();
void updateOledDashboard(int selectedMap, MapProfile profiles[], float currentError, int leftCount, int rightCount);
void runAutoCalibration();
#endif