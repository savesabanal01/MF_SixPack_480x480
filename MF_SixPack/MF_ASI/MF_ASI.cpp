#include "MF_ASI.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite numberTapeSpr(&canvas);
static LGFX_Sprite labelsSpr(&canvas);
static LGFX_Sprite needleSpr(&canvas);

// RunningAverage airSpeedAngleAvg(1);

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_ASI::MF_ASI(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_ASI::begin()
{

}

void MF_ASI::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_RED);
    lcd.setFont(&fonts::Font4);
    delay(3000);
    lcd.fillScreen(TFT_BLUE);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Main_Gauge), ASI_MAIN_GAUGE_WIDTH, ASI_MAIN_GAUGE_HEIGHT, 16);
    numberTapeSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Number_Tape), ASI_NUMBER_TAPE_WIDTH, ASI_NUMBER_TAPE_HEIGHT, 16);
    labelsSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Labels), ASI_LABELS_WIDTH, ASI_LABELS_HEIGHT, 16);
    needleSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Needle), ASI_NEEDLE_WIDTH, ASI_NEEDLE_HEIGHT, 16);
    // ESP Now setup
    // airSpeedAngleAvg.clear();
}

void MF_ASI::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    numberTapeSpr.deleteSprite();
    labelsSpr.deleteSprite();
    needleSpr.deleteSprite();
    lcd.endWrite();
}

void MF_ASI::set(int16_t messageID, char *setPoint)
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
        setAirSpeed(atof(setPoint));
        break;
    case 1:
        /* code */
        setTASRatio(atof(setPoint));
        break;
    case 2:
        /* code */
        break;
    default:
        break;
    }
    drawGauge();
}

void MF_ASI::update()
{
    // Do something which is required regulary
}

void MF_ASI::drawGauge()
{
    rawAngle = calculateAngle(airSpeedFromSim);
    // airSpeedAngleAvg.addValue(rawAngle);
    // angle = airSpeedAngleAvg.getAverage();
    angle = rawAngle;

    TASangle = scaleValue(TASRatio, -1, 1, 15, -95);

    whiteArcStartAngle = calculateAngle(V_S1);
    whiteArcEndAngle = calculateAngle(V_FE);
    greenArcStartAngle = calculateAngle(V_S0);
    greenArcEndAngle = calculateAngle(V_N0);
    yellowArcStartAngle = greenArcEndAngle;
    yellowArcEndAngle = calculateAngle(V_NE);
    V_NEArcStartAngle = yellowArcEndAngle;
    V_NEArcEndAngle = V_NEArcStartAngle + 2;

    startTIme = millis();
    canvas.fillScreen(TFT_BLACK);

    drawLeftGauge();
    drawRightGauge();
}

void MF_ASI::setAirSpeed(float value)
{
    airSpeedFromSim = value;
}

void MF_ASI::setTASRatio(float value)
{
    TASRatio = value;
}

void MF_ASI::drawLeftGauge()
{

    canvas.setPivot(240, 240);
    needleSpr.setPivot(ASI_NEEDLE_WIDTH / 2, 240);

    // Draw left half
    numberTapeSpr.pushSprite(&canvas, 0, 0, TFT_BLUE);
    numberTapeSpr.setPivot(240, 240);
    numberTapeSpr.pushRotated(&canvas, TASangle, TFT_BLUE);
    mainGaugeSpr.pushSprite(&canvas, 0, 0, TFT_BLUE);

    // Draw White Arc

    canvas.fillArc(240, 240, 215, 195, whiteArcStartAngle - 90, whiteArcEndAngle - 90, TFT_WHITE);

    // Draw Green Arc
    canvas.fillArc(240, 240, 195, 169, greenArcStartAngle - 90, greenArcEndAngle - 90, TFT_GREEN);

    // Draw Yellow Arc
    canvas.fillArc(240, 240, 195, 169, yellowArcStartAngle - 90, yellowArcEndAngle - 90, TFT_YELLOW);

    // Draw Red Arc for VNE
    canvas.fillArc(240, 240, 240, 169, V_NEArcStartAngle - 90, V_NEArcEndAngle - 90, TFT_RED);

    // Draw the labels
    labelsSpr.pushSprite(&canvas, 0, 0, TFT_BLACK);

    // Finally, draw the needle
    needleSpr.pushRotated(&canvas, angle, TFT_BLUE);
    canvas.pushSprite(&lcd, 0, 0);
}

void MF_ASI::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240 - x_offset, 240);
    needleSpr.setPivot(ASI_NEEDLE_WIDTH / 2, 240);
    numberTapeSpr.pushRotated(&canvas, TASangle, TFT_BLUE);
    mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, TFT_BLUE);

    // Draw White Arc
    canvas.fillArc(0, 240, 214, 195, whiteArcStartAngle - 90, whiteArcEndAngle - 90, TFT_WHITE);
    // Draw Green Arc
    canvas.fillArc(0, 240, 195, 169, greenArcStartAngle - 90, greenArcEndAngle - 90, TFT_GREEN);
    // Draw Yellow Arc
    canvas.fillArc(0, 240, 195, 169, yellowArcStartAngle - 90, yellowArcEndAngle - 90, TFT_YELLOW);
    // Draw Red Arc for VNE
    canvas.fillArc(0, 240, 240, 169, V_NEArcStartAngle - 90, V_NEArcEndAngle - 90, TFT_RED);
    // Draw the labels
    labelsSpr.pushSprite(&canvas, -x_offset, 0, TFT_BLACK);
    // Finally, draw the needle
    needleSpr.pushRotated(&canvas, angle, TFT_BLUE);

    // Push the canvas sprite to the lcd screen
    canvas.pushSprite(&lcd, x_offset, 0);
}
// Scale Function
float MF_ASI::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Calculate the angle of the needle, etc. based on the air speed because the markers are not linear

float MF_ASI::calculateAngle(float airSpeed)
{
    float calculatedAngle;

    if (airSpeed < 0)
        calculatedAngle = 0;
    // For speeds of 0 to 40 knots
    else if (airSpeed >= 0 and airSpeed < 40)
        calculatedAngle = scaleValue(airSpeed, 0, 40, 0, 30);
    // For speeds of 40 to 60 knots
    else if (airSpeed >= 40 and airSpeed < 60)
        calculatedAngle = scaleValue(airSpeed, 40, 60, 30, 69);
    // For speeds of 60 to 80 knots
    else if (airSpeed >= 60 and airSpeed < 80)
        calculatedAngle = scaleValue(airSpeed, 60, 80, 69, 114);
    // Between 80 and 100 knots
    else if (airSpeed >= 80 and airSpeed < 100)
        calculatedAngle = scaleValue(airSpeed, 80, 100, 114, 164);
    // Between 100 and 120 knots
    else if (airSpeed >= 100 and airSpeed < 120)
        calculatedAngle = scaleValue(airSpeed, 100, 120, 164, 209);
    // Between 120 an d140 knots
    else if (airSpeed >= 120 and airSpeed < 140)
        calculatedAngle = scaleValue(airSpeed, 120, 140, 209, 240);
    // Between 140 and 160 knots
    else if (airSpeed >= 140 and airSpeed < 160)
        calculatedAngle = scaleValue(airSpeed, 140, 160, 240, 269);
    // Between 160 and 180 knots
    else if (airSpeed >= 160 and airSpeed < 180)
        calculatedAngle = scaleValue(airSpeed, 160, 180, 269, 295);
    // Between 180 and 200 knots
    else if (airSpeed >= 180 and airSpeed < 200)
        calculatedAngle = scaleValue(airSpeed, 180, 200, 295, 320);
    // More than 200 knots
    else if (airSpeed >= 200)
        calculatedAngle = 320;

    return calculatedAngle;
}
