#include "MF_VSI.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"
#include "Common_Bezel.h"
#include "RunningAverage.h"
#include "LCDBrightnessTable.h"

#define BACKGROUND_COLOR  0x1041

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
static LGFX_Sprite needleSpr(&canvas);

RunningAverage RA_VSIAngle(5);
int VSIMessageID = -1;

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_VSI::MF_VSI(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_VSI::begin()
{

}

void MF_VSI::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    delay(1000);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(VSI_Main_Gauge), VSI_MAIN_GAUGE_WIDTH, VSI_MAIN_GAUGE_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(Common_Bezel), COMMON_BEZEL_WIDTH, COMMON_BEZEL_HEIGHT, 16);
    needleSpr.setBuffer(const_cast<std::uint16_t *>(VSI_Needle), VSI_NEEDLE_WIDTH, VSI_NEEDLE_HEIGHT, 16);

    RA_VSIAngle.clear();
}

void MF_VSI::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    bezelSpr.deleteSprite();
    needleSpr.deleteSprite();
    lcd.endWrite();
}

void MF_VSI::set(int16_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */
    VSIMessageID = messageID;
    // do something according your messageID
    switch (messageID) {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        break;
    case 0:
        setVerticalSpeed(atof(setPoint));
        break;
   case 100:
        /* code */
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }
}

void MF_VSI::update()
{
    // Do something which is required regulary
    if ( VSIMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
    {
        lcd.fillScreen(TFT_BLACK);
        analogWrite(BACKLIGHT_PIN, 0);
    }
    else
    {
        float pwmOutput = 0;
        pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
        analogWrite(BACKLIGHT_PIN, pwmOutput);
        drawGauge();
    }
}

void MF_VSI::drawGauge()
{
    VSIAngle = scaleValue(verticalSpeed, -2000, 2000, -170, 170); // The needle starts at -90 degrees
    RA_VSIAngle.addValue(VSIAngle);
    canvas.fillScreen(TFT_BLACK);

    drawLeftGauge();
    drawRightGauge();
}

void MF_VSI::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.setPivot(240, 240);
    needleSpr.setPivot(198, VSI_NEEDLE_HEIGHT / 2);
    mainGaugeSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    needleSpr.pushRotated(&canvas, RA_VSIAngle.getAverage(), BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);

    canvas.pushSprite(&lcd, 0, 0);

}

void MF_VSI::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240 - x_offset, 240);
    needleSpr.setPivot(198, VSI_NEEDLE_HEIGHT / 2);
    mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
    needleSpr.pushRotated(&canvas, RA_VSIAngle.getAverage(), BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, x_offset, 0);
}

// Setters
void MF_VSI::setVerticalSpeed(float value)
{
    verticalSpeed = value;
}

void MF_VSI::setPowerSave(bool enabled)
{
    if (enabled) {
        powerSaveFlag = true;
    } else {
        powerSaveFlag = false;
    }
}

void MF_VSI::setInstrumentBrightness(float value)
{
    float pwmOutput = 0;

    instrumentBrightness = scaleValue(value, 0, 1, 100, 255);
    pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
    analogWrite(BACKLIGHT_PIN, pwmOutput);
}

// Scale Function
float MF_VSI::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
