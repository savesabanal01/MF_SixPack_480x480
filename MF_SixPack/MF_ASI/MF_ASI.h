#pragma once

#include <Arduino.h>
#include "include/ASI_Labels.h"
#include "include/ASI_Main_Gauge.h"
#include "include/ASI_Needle.h"
#include "include/ASI_Number_Tape.h"

class MF_ASI
{
public:
    MF_ASI(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:
    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;

 // Variables
    float rawAngle = 0;
    float angle = 0;
    float TASangle = 0;
    uint16_t x_offset = 240;
    float startTIme = 0;
    float endTime = 0;
    float airSpeedFromSim = 0;
    float TASRatio = 0;
    bool powerSaveFlag = false;
    uint8_t instrumentBrightness = 255;

    // Cessna 172 Default V Speeds
    uint16_t V_S0 = 48;
    uint16_t V_S1 = 40;
    uint16_t V_FE = 85;
    uint16_t V_NO = 129;
    uint16_t V_NE = 163;

    uint16_t whiteArcStartAngle;
    uint16_t whiteArcEndAngle;
    uint16_t greenArcStartAngle;
    uint16_t greenArcEndAngle;
    uint16_t yellowArcStartAngle;
    uint16_t yellowArcEndAngle;
    uint16_t V_NEArcStartAngle;
    uint16_t V_NEArcEndAngle;

    uint16_t BACKLIGHT_PIN = 38;

    // Functions
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void drawGauge();
    void drawLeftGauge();
    void drawRightGauge();
    float calculateAngle(float airSpeed);
    void setAirSpeed(float value);
    void setTASRatio(float value);
    void setVS0(float value);
    void setVS1(float value);
    void setVFE(float value);
    void setVNO(float value);
    void setVNE(float value);
    void setPowerSave(bool enabled);
    void setInstrumentBrightness(float value);
};