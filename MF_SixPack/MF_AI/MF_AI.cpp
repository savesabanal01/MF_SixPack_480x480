#include "MF_AI.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"

#define BACKGROUND_COLOR  0x1041
#define SKYBLUE 0x5DDC

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite bezelSpr(&canvas);
static LGFX_Sprite rollIndicatorSpr(&canvas);
static LGFX_Sprite pitchIndicatorSpr(&canvas);
// RunningAverage airSpeedAngleAvg(1);

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_AI::MF_AI(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_AI::begin()
{

}

void MF_AI::attach(uint16_t Pin3, char *init)
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
    bezelSpr.setBuffer(const_cast<std::uint16_t *>(AI_Bezel), AI_BEZEL_WIDTH, AI_BEZEL_HEIGHT, 16);
    rollIndicatorSpr.setBuffer(const_cast<std::uint16_t *>(AI_Roll_Indicator), AI_ROLL_INDICATOR_WIDTH, AI_ROLL_INDICATOR_HEIGHT, 16);
    pitchIndicatorSpr.setBuffer(const_cast<std::uint16_t *>(AI_Pitch_Indicator), AI_PITCH_INDICATOR_WIDTH, AI_PITCH_INDICATOR_HEIGHT, 16);
}

void MF_AI::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    bezelSpr.deleteSprite();
    rollIndicatorSpr.deleteSprite();
    pitchIndicatorSpr.deleteSprite();
    lcd.fillScreen(TFT_BLACK);
    lcd.endWrite();
}

void MF_AI::set(int16_t messageID, char *setPoint)
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
        setPitchAngle(atof(setPoint));
        break;
    case 1:
        setRollAngle(atof(setPoint));
        break;
    case 2:
        /* code */
        break;
    default:
        break;
    }
    drawGauge();
}

void MF_AI::update()
{
    // Do something which is required regulary
}

void MF_AI::drawGauge()
{
    if (pitchAngle > 30)
        pitchAngle = 30;
    else if (pitchAngle < -30)
        pitchAngle = -30;

    pitchIndicatorPosition= scaleValue(pitchAngle, -30, 30, -90, 90); // The needle starts at -90 degrees

    drawLeftGauge();
    drawRightGauge();
}

void MF_AI::setRollAngle(float value)
{
    rollAngle = value;
}

void MF_AI::setPitchAngle(float value)
{
    pitchAngle = value;
}


void MF_AI::drawLeftGauge()
{
    // Draw Left Half of VSI Gauge
    canvas.fillScreen(SKYBLUE);
    canvas.setPivot(240, 240);
    rollIndicatorSpr.setPivot(240, 240);
    pitchIndicatorSpr.setPivot(AI_PITCH_INDICATOR_WIDTH/2, AI_PITCH_INDICATOR_HEIGHT/2 - pitchIndicatorPosition + 4);
    pitchIndicatorSpr.pushRotated(&canvas, -rollAngle, BACKGROUND_COLOR);
    rollIndicatorSpr.pushRotated(&canvas, -rollAngle, BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, 0, 0);

}

void MF_AI::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(SKYBLUE);
    canvas.setPivot(240 - x_offset, 240);
    rollIndicatorSpr.setPivot(240, 240);
    pitchIndicatorSpr.setPivot(AI_PITCH_INDICATOR_WIDTH/2, AI_PITCH_INDICATOR_HEIGHT/2 - pitchIndicatorPosition + 4);
    pitchIndicatorSpr.pushRotated(&canvas, -rollAngle, BACKGROUND_COLOR);
    rollIndicatorSpr.pushRotated(&canvas, -rollAngle, BACKGROUND_COLOR);
    bezelSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, x_offset, 0);
}

// Scale Function
float MF_AI::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
