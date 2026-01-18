#include "MF_TC.h"
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
static LGFX_Sprite planeSpr(&canvas);
static LGFX_Sprite ballSpr(&canvas);
static LGFX_Sprite markerSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);

RunningAverage RA_TurnAngle(5);
RunningAverage RA_SlipAngle(5);

int TCMessageID = -1;

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_TC::MF_TC(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_TC::begin()
{

}

void MF_TC::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    delay(1000);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(TC_Main_Gauge), TC_MAIN_GAUGE_WIDTH, TC_MAIN_GAUGE_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(Common_Bezel), COMMON_BEZEL_WIDTH, COMMON_BEZEL_HEIGHT, 16);
    planeSpr.setBuffer(const_cast<std::uint16_t *>(TC_Plane), TC_PLANE_WIDTH, TC_PLANE_HEIGHT, 16);
    ballSpr.setBuffer(const_cast<std::uint16_t *>(TC_Ball), TC_BALL_WIDTH, TC_BALL_HEIGHT, 16);
    markerSpr.setBuffer(const_cast<std::uint16_t *>(TC_Marker), TC_MARKER_WIDTH, TC_MARKER_HEIGHT, 16);
}

void MF_TC::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    bezelSpr.deleteSprite();
    planeSpr.deleteSprite();
    ballSpr.deleteSprite();
    markerSpr.deleteSprite();
    lcd.endWrite();
}

void MF_TC::set(int16_t messageID, char *setPoint)
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
   TCMessageID = messageID;

    // do something according your messageID
    switch (messageID) {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        break;
    case 0:
        setTurnAngle(atof(setPoint));
        break;
    case 1:
        /* code */
        setSlipAngle(atof(setPoint));
        break;
    case 100:
        /* code */
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }
}

void MF_TC::update()
{
    // Do something which is required regulary
    if (TCMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
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

void MF_TC::drawGauge()
{

    RA_TurnAngle.addValue(turnAngle);
    RA_SlipAngle.addValue(slipAngle);

    canvas.fillScreen(TFT_BLACK);
    ballXPos = (int)round(scaleValue(RA_SlipAngle.getAverage(), 8, -8, 120, 360 - TC_BALL_WIDTH));
    ballYPos = 260 + (int)(56 * sqrt((1 - (((ballXPos - 190) * (ballXPos - 190) / (190 * 190)))))); // Approximation based on Ellipse equation
    drawLeftGauge();
    drawRightGauge();
}

void MF_TC::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 256);

    mainGaugeSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    planeSpr.setPivot(TC_PLANE_WIDTH / 2, 56);
    planeSpr.pushRotated(&canvas, RA_TurnAngle.getAverage(), BACKGROUND_COLOR);
    ballSpr.pushSprite(&canvas, ballXPos, ballYPos, BACKGROUND_COLOR);
    markerSpr.pushSprite(&canvas, 208, 313, BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, 0, 0);

}

void MF_TC::drawRightGauge()
{
    // Draw right half
  canvas.fillScreen(TFT_BLACK);
  canvas.setPivot(240 - x_offset, 256);
  mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
  planeSpr.setPivot(TC_PLANE_WIDTH / 2, 56);
  planeSpr.pushRotated(&canvas, RA_TurnAngle.getAverage(), BACKGROUND_COLOR);
  ballSpr.pushSprite(&canvas, ballXPos - x_offset, ballYPos, BACKGROUND_COLOR);
  markerSpr.pushSprite(&canvas, 208 - x_offset, 313, BACKGROUND_COLOR);
  bezelSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
  canvas.pushSprite(&lcd, x_offset, 0);
}

// Setter
void MF_TC::setTurnAngle(float value)
{
    turnAngle = value;
}

void MF_TC::setSlipAngle(float value)
{
    slipAngle = value;
}

void MF_TC::setPowerSave(bool enabled)
{
    powerSaveFlag = enabled;  
}

void MF_TC::setInstrumentBrightness(float value)
{
    uint8_t pwmOutput = 0;

    instrumentBrightness = scaleValue(value, 0, 1, 100, 255);
    pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
    analogWrite(BACKLIGHT_PIN, pwmOutput);
}

// Scale Function
float MF_TC::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
