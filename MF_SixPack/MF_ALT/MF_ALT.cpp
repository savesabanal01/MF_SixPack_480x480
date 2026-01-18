#include "MF_ALT.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"
#include "LCDBrightnessTable.h"
#include "RunningAverage.h"
#include "math.h"

#define BACKGROUND_COLOR  0x1041

#define INHG_TO_HPA 33.8639

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite baroInHgSpr(&canvas);
static LGFX_Sprite baroHpaSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
static LGFX_Sprite needle100Spr(&canvas);
static LGFX_Sprite needle1000Spr(&canvas);
static LGFX_Sprite needle10000Spr(&canvas);

int ALTMessageID = -1;

RunningAverage RA_Altitude(5);
RunningAverage RA_Baro(5);
RunningAverage RA_BaroHpa(5);


/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_ALT::MF_ALT(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_ALT::begin()
{

}

void MF_ALT::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_BLACK);
    delay(1000);
    lcd.setFont(&fonts::Font4);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Main_Gauge), ALT_MAIN_GAUGE_WIDTH, ALT_MAIN_GAUGE_HEIGHT, 16);
    baroInHgSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Baro_InHg), ALT_BARO_INHG_WIDTH, ALT_BARO_INHG_HEIGHT, 16);
    baroHpaSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Baro_Hpa), ALT_BARO_HPA_WIDTH, ALT_BARO_HPA_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Bezel), ALT_BEZEL_WIDTH, ALT_BEZEL_HEIGHT, 16);
    needle100Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_100), ALT_NEEDLE_100_WIDTH, ALT_NEEDLE_100_HEIGHT, 16);
    needle1000Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_1000), ALT_NEEDLE_1000_WIDTH, ALT_NEEDLE_1000_HEIGHT, 16);
    needle10000Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_10000), ALT_NEEDLE_10000_WIDTH, ALT_NEEDLE_10000_HEIGHT, 16);

    RA_Altitude.clear();
    RA_Baro.clear();
    RA_BaroHpa.clear();

}


void MF_ALT::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    bezelSpr.deleteSprite();
    baroInHgSpr.deleteSprite();
    needle100Spr.deleteSprite();
    needle1000Spr.deleteSprite();
    needle10000Spr.deleteSprite();
    lcd.endWrite();
}

void MF_ALT::set(int16_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */;
    ALTMessageID = messageID;
    // do something according your messageID
    switch (messageID) {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        setPowerSave(atoi(setPoint));
        break;
    case 0:
        setAltitude(atof(setPoint));
        break;
    case 1:
        /* code */
        setBaro(atof(setPoint));
        break;
    case 100:
        /* code */
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }
}

void MF_ALT::update()
{
    // Do something which is required regulary
    if (ALTMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
    {
        lcd.fillScreen(TFT_BLACK);
        canvas.fillSprite(TFT_BLACK);
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

void MF_ALT::drawGauge()
{
    baroHpa = baro * INHG_TO_HPA;

    RA_Altitude.addValue(altitude);
    canvas.fillScreen(TFT_BLACK);
    thousand = fmod(RA_Altitude.getAverage(), 10000);
    hundred = fmod(RA_Altitude.getAverage(), 1000);
    needle10000Angle = scaleValue(RA_Altitude.getAverage(), 0, 100000, 0, 360);
    needle1000Angle = scaleValue(thousand, 0, 10000, 0, 360);
    needle100Angle = scaleValue(hundred, 0, 1000, 0, 360);

    RA_Baro.addValue(baro);
    baroAngle = scaleValue(RA_Baro.getAverage(), 28.2, 31.6, 170, -170);

    RA_BaroHpa.addValue(baroHpa);
    baroHpaAngle = scaleValue(RA_BaroHpa.getAverage(), 925, 1095, 170, -170);


    drawLeftGauge();
    drawRightGauge();
    // lcd.setTextColor(TFT_GREEN);
    // // canvas.setTextFont(7);
    // lcd.setTextSize(1);
    // lcd.setCursor(100, 100);
    // lcd.println(String(instrumentBrightness));
}

void MF_ALT::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 240);

    baroHpaSpr.setPivot(240, 240);
    baroHpaSpr.pushRotated(&canvas, baroHpaAngle);

    mainGaugeSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);

    needle10000Spr.setPivot(ALT_NEEDLE_10000_WIDTH / 2, 233);
    needle10000Spr.pushRotated(&canvas, needle10000Angle, BACKGROUND_COLOR);

    needle1000Spr.setPivot(ALT_NEEDLE_1000_WIDTH / 2, 133);
    needle1000Spr.pushRotated(&canvas, needle1000Angle, BACKGROUND_COLOR);

    needle100Spr.setPivot(ALT_NEEDLE_100_WIDTH / 2, 221);
    needle100Spr.pushRotated(&canvas, needle100Angle, BACKGROUND_COLOR);

    bezelSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);

    canvas.pushSprite(&lcd, 0, 0);

}

void MF_ALT::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240 - x_offset, 240);
    baroInHgSpr.setPivot(240, 240);
    baroInHgSpr.pushRotated(&canvas, baroAngle);

    mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);

    needle10000Spr.setPivot(ALT_NEEDLE_10000_WIDTH / 2, 233);
    needle10000Spr.pushRotated(&canvas, needle10000Angle, BACKGROUND_COLOR);

    needle1000Spr.setPivot(ALT_NEEDLE_1000_WIDTH / 2, 133);
    needle1000Spr.pushRotated(&canvas, needle1000Angle, BACKGROUND_COLOR);

    needle100Spr.setPivot(ALT_NEEDLE_100_WIDTH / 2, 221);
    needle100Spr.pushRotated(&canvas, needle100Angle, BACKGROUND_COLOR);

    bezelSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);

    canvas.pushSprite(&lcd, x_offset, 0);
}

// Setters
void MF_ALT::setAltitude(float value)
{
    altitude = value;
}

void MF_ALT::setBaro(float value)
{
    baro = value;
}

void MF_ALT::setPowerSave(bool enabled)
{
    powerSaveFlag = enabled;
}


void MF_ALT::setInstrumentBrightness(float value)
{
    float pwmOutput = 0;

    instrumentBrightness = scaleValue(value, 0, 1, 100, 255);
    pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
    analogWrite(BACKLIGHT_PIN, pwmOutput);
}

// Scale Function
float MF_ALT::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
