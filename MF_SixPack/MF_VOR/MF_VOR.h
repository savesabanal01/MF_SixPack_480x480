#pragma once

#include <Arduino.h>
#include "include/HI_Heading_Tape.h"
#include "include/HI_Main_Gauge.h"

class MF_VOR
{
public:
    MF_VOR(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:
    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;

    // Variables
    float heading = 0.0f;

    // NAV indicators (for Mobiflight naming / X-Plane inputs)
    // VORind: CDI (left/right) deviation in "dots" (typical range ~ -2.5 .. +2.5)
    // GSind:  Glideslope (up/down) deviation in "dots" (typical range ~ -2.5 .. +2.5)
    float VORind = 0.0f;
    float GSind  = 0.0f;

    // Flags
    // TOFROMind: -1 = FROM, 0 = OFF, +1 = TO
    float TOFROMind = 0.0f;
    // NAVflag: 1 = show NAV flag (signal invalid), 0 = hide
    float NAVflag = 0.0f;

    uint16_t x_offset = 240;
    bool powerSaveFlag = false;
    uint8_t instrumentBrightness = 255;

    uint16_t BACKLIGHT_PIN = 38;
    float headingAverage = 0.0f;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    float clampf(float v, float lo, float hi);

    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();

    void drawNavIndicators(int16_t x_shift);
    // void drawFlags(int16_t x_shift);

    void setHeading(float value);
    void setVORind(float value);
    void setGSind(float value);
    void setTOFROMind(float value);
    void setNAVflag(float value);

    void setPowerSave(bool enabled);
    void setInstrumentBrightness(float value);
};
