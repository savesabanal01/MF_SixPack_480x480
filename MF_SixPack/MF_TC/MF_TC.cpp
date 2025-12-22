#include "MF_TC.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite planeSpr(&canvas);
static LGFX_Sprite ballSpr(&canvas);
static LGFX_Sprite markerSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
// RunningAverage airSpeedAngleAvg(1);

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

    lcd.fillScreen(TFT_GREEN);
    lcd.setFont(&fonts::Font4);
    delay(3000);
    lcd.fillScreen(TFT_YELLOW);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(TC_Main_Gauge), TC_MAIN_GAUGE_WIDTH, TC_MAIN_GAUGE_HEIGHT, 16);
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(TC_Bezel), TC_BEZEL_WIDTH, TC_BEZEL_HEIGHT, 16);
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
        setTurnAngle(atof(setPoint));
        break;
    case 1:
        /* code */
        setSlipAngle(atof(setPoint));
        break;
    case 2:
        /* code */
        break;
    default:
        break;
    }
    drawGauge();
}

void MF_TC::update()
{
    // Do something which is required regulary
}

void MF_TC::drawGauge()
{
    // VSIAngle = scaleValue(verticalSpeed, -2000, 2000, -170, 170); // The needle starts at -90 degrees

    canvas.fillScreen(TFT_BLACK);
    ballXPos = (int)round(scaleValue(slipAngle, 8, -8, 120, 360 - TC_BALL_WIDTH));
    ballYPos = 260 + (int)(56 * sqrt((1 - (((ballXPos - 190) * (ballXPos - 190) / (190 * 190)))))); // Approximation based on Ellipse equation
    drawLeftGauge();
    drawRightGauge();
}

void MF_TC::setTurnAngle(float value)
{
    turnAngle = value;
}

void MF_TC::setSlipAngle(float value)
{
    slipAngle = value;
}



void MF_TC::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge

    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 256);

    mainGaugeSpr.pushSprite(&canvas, 0, 0, TFT_BLACK);
    planeSpr.setPivot(TC_PLANE_WIDTH / 2, 56);
    planeSpr.pushRotated(&canvas, turnAngle, TFT_BLUE);
    ballSpr.pushSprite(&canvas, ballXPos, ballYPos, TFT_BLUE);
    markerSpr.pushSprite(&canvas, 208, 312, TFT_BLUE);
    bezelSpr.pushSprite(&canvas, 0, 0, TFT_BLUE);
    canvas.pushSprite(&lcd, 0, 0);

}

void MF_TC::drawRightGauge()
{
    // Draw right half
  canvas.fillScreen(TFT_BLACK);
  canvas.setPivot(240 - x_offset, 256);
  mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, TFT_BLUE);
  planeSpr.setPivot(TC_PLANE_WIDTH / 2, 56);
  planeSpr.pushRotated(&canvas, (int)round(turnAngle), TFT_BLUE);
  ballSpr.pushSprite(&canvas, ballXPos - x_offset, ballYPos, TFT_BLUE);
  markerSpr.pushSprite(&canvas, 208 - x_offset, 312, TFT_BLUE);
  bezelSpr.pushSprite(&canvas, -x_offset, 0, TFT_BLUE);
  canvas.pushSprite(&lcd, x_offset, 0);
}

// Scale Function
float MF_TC::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
