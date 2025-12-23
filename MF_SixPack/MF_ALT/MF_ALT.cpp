#include "MF_ALT.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"

#define BACKGROUND_COLOR  0x1041

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite baroSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
static LGFX_Sprite needle100Spr(&canvas);
static LGFX_Sprite needle1000Spr(&canvas);
static LGFX_Sprite needle10000Spr(&canvas);
// RunningAverage airSpeedAngleAvg(1);

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

    lcd.fillScreen(TFT_GREEN);
    lcd.setFont(&fonts::Font4);
    delay(3000);
    lcd.fillScreen(TFT_YELLOW);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Main_Gauge), ALT_MAIN_GAUGE_WIDTH, ALT_MAIN_GAUGE_HEIGHT, 16);
    baroSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Baro_InHg), ALT_BARO_INHG_WIDTH, ALT_BARO_INHG_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(ALT_Bezel), ALT_BEZEL_WIDTH, ALT_BEZEL_HEIGHT, 16);
    needle100Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_100), ALT_NEEDLE_100_WIDTH, ALT_NEEDLE_100_HEIGHT, 16);
    needle1000Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_1000), ALT_NEEDLE_1000_WIDTH, ALT_NEEDLE_1000_HEIGHT, 16);
    needle10000Spr.setBuffer(const_cast<std::uint16_t *>(ALT_Needle_10000), ALT_NEEDLE_10000_WIDTH, ALT_NEEDLE_10000_HEIGHT, 16);

}

void MF_ALT::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    bezelSpr.deleteSprite();
    baroSpr.deleteSprite();
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
        setAltitude(atof(setPoint));
        break;
    case 1:
        /* code */
        setBaro(atof(setPoint));
        break;
    case 2:
        /* code */
        break;
    default:
        break;
    }
    drawGauge();
}

void MF_ALT::update()
{
    // Do something which is required regulary
}

void MF_ALT::drawGauge()
{
    // VSIAngle = scaleValue(verticalSpeed, -2000, 2000, -170, 170); // The needle starts at -90 degrees

    canvas.fillScreen(TFT_BLACK);
    thousand = (int)altitude % 10000;
    hundred = (int)altitude % 1000;
    needle10000Angle = scaleValue(altitude, 0, 10000, 0, 360);
    needle1000Angle = scaleValue(thousand, 0, 1000, 0, 360);
    needle100Angle = scaleValue(hundred, 0, 100, 0, 360);
    baroAngle = scaleValue(baro, 31.1, 28.6, -131, 131);
    
    drawLeftGauge();
    drawRightGauge();
}

void MF_ALT::setAltitude(float value)
{
    altitude = value;
}

void MF_ALT::setBaro(float value)
{
    baro = value;
}


void MF_ALT::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 240);
    baroSpr.setPivot(240, 240);
    baroSpr.pushRotated(&canvas, baroAngle);

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
    baroSpr.setPivot(240, 240);
    baroSpr.pushRotated(&canvas, baroAngle);

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

// Scale Function
float MF_ALT::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
