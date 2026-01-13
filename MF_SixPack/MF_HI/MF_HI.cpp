#include "MF_HI.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"
#include "Common_Bezel.h"

#define BACKGROUND_COLOR  0x1041

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite headingTapeSpr(&canvas);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite needleSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
// RunningAverage airSpeedAngleAvg(1);

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_HI::MF_HI(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_HI::begin()
{

}

void MF_HI::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_RED);
    lcd.setFont(&fonts::Font4);
    delay(1000);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(HI_Main_Gauge), HI_MAIN_GAUGE_WIDTH, HI_MAIN_GAUGE_HEIGHT, 16);
    headingTapeSpr.setBuffer(const_cast<std::uint16_t *>(HI_Heading_Tape), HI_HEADING_TAPE_WIDTH, HI_HEADING_TAPE_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(Common_Bezel), COMMON_BEZEL_WIDTH, COMMON_BEZEL_HEIGHT, 16);
    needleSpr.setBuffer(const_cast<std::uint16_t *>(HI_Needle), HI_NEEDLE_WIDTH, HI_NEEDLE_HEIGHT, 16);
}

void MF_HI::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    bezelSpr.deleteSprite();
    needleSpr.deleteSprite();
    headingTapeSpr.deleteSprite();
    lcd.endWrite();
}

void MF_HI::set(int16_t messageID, char *setPoint)
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
    // int32_t  data = atoi(setPoint);
    // uint16_t output;

    // do something according your messageID
    switch (messageID) {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        break;
    case 0:
        setHeading(atof(setPoint));
        break;
    case 1:
        /* code */
        setHeadingBug(atof(setPoint));
        break;
    case 2:
        /* code */
        break;
    default:
        break;
    }

}

void MF_HI::update()
{
    // Do something which is required regulary
    drawGauge();
}

void MF_HI::drawGauge()
{
    // VSIAngle = scaleValue(verticalSpeed, -2000, 2000, -170, 170); // The needle starts at -90 degrees

    canvas.fillScreen(TFT_BLACK);

    drawLeftGauge();
    drawRightGauge();
}

void MF_HI::setHeading(float value)
{
    heading = value;
}

void MF_HI::setHeadingBug(float value)
{
    headingBug = value;
}


void MF_HI::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 240);

    headingTapeSpr.setPivot(240, 240);
    headingTapeSpr.pushRotated(&canvas, heading, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, 90, 90, BACKGROUND_COLOR);
    needleSpr.setPivot(HI_NEEDLE_WIDTH/2, 235);
    needleSpr.pushRotated(&canvas, headingBug, BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, 0, 0);

}

void MF_HI::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(TFT_BLACK);
    headingTapeSpr.setPivot(240, 240);
    canvas.setPivot(240 - x_offset, 240);
    headingTapeSpr.pushRotated(&canvas, heading, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, 90 - x_offset, 90, BACKGROUND_COLOR);
    needleSpr.setPivot(HI_NEEDLE_WIDTH/2, 235);
    needleSpr.pushRotated(&canvas, headingBug, BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, x_offset, 0);
}

// Scale Function
float MF_HI::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
