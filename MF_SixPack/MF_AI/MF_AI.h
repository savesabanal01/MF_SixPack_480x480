#pragma once

#include <Arduino.h>
#include "include/AI_Bezel.h"
#include "include/AI_Pitch_Indicator.h"
#include "include/AI_Roll_Indicator.h"


class MF_AI
{
public:
    MF_AI(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
 // Variables
    float rollAngle = 0;
    float pitchAngle = 0;
    float pitchIndicatorPosition = 0;
    float rollIndicatorPosition = 0;
    uint16_t x_offset = 240;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    void setPitchAngle(float value);
    void setRollAngle(float value);
};