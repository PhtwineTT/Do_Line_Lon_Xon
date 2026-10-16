#include "sensors.h"

// MUX
int muxChannelMap[NUM_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
int rawSensor[NUM_SENSORS];
int minVal[NUM_SENSORS];
int maxVal[NUM_SENSORS];
int normSensor[NUM_SENSORS];

// Trạng thái sensors
bool isAllWhite = true;
int leftBlackCount = 0;
int rightBlackCount = 0;
float currentError = 0;

// Khởi tạo
void setupSensors()
{
    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    pinMode(MUX_S3, OUTPUT);
    resetCalibData();
    analogReadResolution(10);
    analogSetAttenuation(ADC_11db);
}

// Chọn kênh MUX
void selectMuxChannel(int ch)
{
    digitalWrite(MUX_S0, (ch >> 0) & 0x01);
    digitalWrite(MUX_S1, (ch >> 1) & 0x01);
    digitalWrite(MUX_S2, (ch >> 2) & 0x01);
    digitalWrite(MUX_S3, (ch >> 3) & 0x01);
}

// Đọc sensors
void readAllRawSensors()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        selectMuxChannel(muxChannelMap[i]);
        delayMicroseconds(2);
        rawSensor[i] = analogRead(MUX_SIG);
    }
}

// Hiệu chuẩn
void resetCalibData()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        minVal[i] = 1023;
        maxVal[i] = 0;
    }
}

void readCalibratedSensors()
{
    readAllRawSensors();
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int range = maxVal[i] - minVal[i];
        if (range < 50)
            range = 50;
        long val = (long)(rawSensor[i] - minVal[i]) * 1000 / range;
        val = constrain(val, 0, 1000);
        if (INVERT_SENSORS)
            val = 1000 - val;
        normSensor[i] = (int)val;
    }
}

// Phân tích dữ liệu
void analyzeLine()
{
    readCalibratedSensors();
    long weightedSum = 0;
    long sum = 0;
    isAllWhite = true;
    leftBlackCount = 0;
    rightBlackCount = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int val = normSensor[i];
        if (val > 250)
        {
            isAllWhite = false;
            int weight = (int)((i - 7.5f) * 1000.0f);
            weightedSum += (long)val * weight;
            sum += val;
        }
        if (val > 600)
        {
            if (i <= 4)
                leftBlackCount++;
            if (i >= 11)
                rightBlackCount++;
        }
    }

    if (!isAllWhite && sum > 0)
    {
        float newError = (float)weightedSum / sum;
        if (!(abs(currentError) > 5500 && abs(newError - currentError) > 6000))
            currentError = newError;
    }
}