#include "display.h"

void setupDisplay() {
    Serial.println("===========================");
    Serial.println("SERIAL DEBUG");
    Serial.println("===========================");
}

void updateOledDashboard(int selectedMap, MapProfile profiles[], float currentError, int leftCount, int rightCount) {
    Serial.print("MAP: "); Serial.print(profiles[selectedMap].name);
    Serial.print(" | Kp: "); Serial.print(profiles[selectedMap].kp, 3);
    Serial.print(" | Kd: "); Serial.print(profiles[selectedMap].kd, 2);
    Serial.print(" | Err: "); Serial.print(currentError, 0);
    Serial.print(" | Black(L,R): "); Serial.print(leftCount); 
    Serial.print(","); Serial.println(rightCount);
}

void runAutoCalibration() {
    resetCalibData();
    unsigned long start = millis();
    unsigned long lastPrint = 0;

    Serial.println(">>> BAT DAU CALIB (Quet dau xe qua vach) <<<");

    while (millis() - start < 4000) {
        readAllRawSensors();
        for (int i = 0; i < NUM_SENSORS; i++) {
            if (rawSensor[i] < minVal[i]) minVal[i] = rawSensor[i];
            if (rawSensor[i] > maxVal[i]) maxVal[i] = rawSensor[i];
        }

        if (millis() - lastPrint >= 500) {
            lastPrint = millis();
            Serial.print("Calibrating... ");
            Serial.print(map(millis() - start, 0, 4000, 0, 100));
            Serial.println("%");
        }
    }
    Serial.println(">>> CALIB XONG! <<<");
}