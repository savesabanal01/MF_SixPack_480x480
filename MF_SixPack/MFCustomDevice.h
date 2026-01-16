#pragma once

#include <Arduino.h>
#include <MF_ASI/MF_ASI.h>
#include <MF_HI/MF_HI.h>
#include <MF_TC/MF_TC.h>
#include <MF_ALT/MF_ALT.h>

// only one entry required if you have only one custom device
enum {
    MF_ASI_DEVICE = 1,
    MF_HI_DEVICE,
    MF_TC_DEVICE,
    MF_ALT_DEVICE
};
class MFCustomDevice
{
public:
    MFCustomDevice();
    void attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash = false);
    void detach();
    void update();
    void set(int16_t messageID, char *setPoint);

private:
    bool           getStringFromMem(uint16_t addreeprom, char *buffer, bool configFromFlash);
    bool           _initialized = false;
    MF_ASI         *_myASIdevice;
    MF_HI          *_myHIdevice;
    MF_TC          *_myTCdevice;
    MF_ALT         *_myALTdevice;
    uint8_t        _pin1, _pin2, _pin3;
    uint8_t        _customType = 0;
};
