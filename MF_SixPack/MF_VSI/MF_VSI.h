#pragma once

#include <Arduino.h>
#include "include/VSI_Main_Gauge.h"
#include "include/VSI_Needle.h"


class MF_VSI
{
public:
    MF_VSI(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
 // Variables
    float verticalSpeed = 0;
    float VSIAngle = 0;
    uint16_t x_offset = 240;
    uint16_t BACKLIGHT_PIN = 38;
    bool powerSaveFlag = false;
    uint8_t instrumentBrightness = 255;


    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    void setVerticalSpeed(float value);
    void setPowerSave(bool enabled);
    void setInstrumentBrightness(float value);
};