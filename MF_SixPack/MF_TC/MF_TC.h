#pragma once

#include <Arduino.h>
#include "include/TC_Ball.h"
#include "include/TC_Main_Gauge.h"
#include "include/TC_Marker.h"
#include "include/TC_Plane.h"


class MF_TC
{
public:
    MF_TC(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
 // Variables
    float turnAngle = 0;
    float slipAngle = 0;
    int ballXPos = 0;
    float ballYPos = 0;
    uint16_t x_offset = 240;
    bool powerSaveFlag = false;
    uint8_t instrumentBrightness = 255;

    uint16_t BACKLIGHT_PIN = 38;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    void setTurnAngle(float value);
    void setSlipAngle (float value);
    void setPowerSave(bool enabled);
    void setInstrumentBrightness(float value);
};