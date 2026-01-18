#pragma once

#include <Arduino.h>
#include "include/HI_Heading_Tape.h"
#include "include/HI_Main_Gauge.h"
#include "include/HI_Needle.h"

class MF_HI
{
public:
    MF_HI(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
 // Variables
    int heading = 0;
    int headingBug = 0;
    uint16_t x_offset = 240;
    bool powerSaveFlag = false;
    uint8_t instrumentBrightness = 255;

    uint16_t BACKLIGHT_PIN = 38;
    float headingAverage = 0;
    float headingBugAverage = 0;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    void setHeading(float value);    // angle for heading
    void setHeadingBug(float value); // angle for heading bug
    void setPowerSave(bool enabled);
    void setInstrumentBrightness(float value);
};