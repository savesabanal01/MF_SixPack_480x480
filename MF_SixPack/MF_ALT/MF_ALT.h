#pragma once

#include <Arduino.h>
#include "include/ALT_Baro_InHg.h"
#include "include/ALT_Bezel.h"
#include "include/ALT_Main_Gauge.h"
#include "include/ALT_Needle_100.h"
#include "include/ALT_Needle_1000.h"
#include "include/ALT_Needle_10000.h"


class MF_ALT
{
public:
    MF_ALT(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:

    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
 // Variables
    float altitude = 0;   // altitude value from sim
    float baro = 0;     // baro value from sim
    float baroAngle = 0;
    float needle100Angle = 0;
    float needle1000Angle = 0;
    float needle10000Angle = 0;
    float thousand = 0; // thousand value for the needle1000
    float hundred = 0;  // hundred value for the needle100
    uint16_t x_offset = 240;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    void setAltitude(float value);    // angle for heading
    void setBaro(float value); // angle for heading bug
};