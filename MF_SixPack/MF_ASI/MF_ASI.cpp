#include "MF_ASI.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"
#include "RunningAverage.h"
#include "LCDBrightnessTable.h"

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite mainGaugeSpr(&canvas);
static LGFX_Sprite numberTapeSpr(&canvas);
static LGFX_Sprite labelsSpr(&canvas);
static LGFX_Sprite needleSpr(&canvas);

#define BACKGROUND_COLOR  0x1041

RunningAverage RA_Airspeed(5);
RunningAverage RA_TASKnob(5);

int ASIMessageID = -1;

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

    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    lcd.fillScreen(BACKGROUND_COLOR);

    canvas.createSprite(240, 480);
    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Main_Gauge), ASI_MAIN_GAUGE_WIDTH, ASI_MAIN_GAUGE_HEIGHT, 16);
    numberTapeSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Number_Tape), ASI_NUMBER_TAPE_WIDTH, ASI_NUMBER_TAPE_HEIGHT, 16);
    labelsSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Labels), ASI_LABELS_WIDTH, ASI_LABELS_HEIGHT, 16);
    needleSpr.setBuffer(const_cast<std::uint16_t *>(ASI_Needle), ASI_NEEDLE_WIDTH, ASI_NEEDLE_HEIGHT, 16);

    RA_Airspeed.clear();
    RA_TASKnob.clear();
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
    ASIMessageID = messageID;

    // do something according your messageID
    switch (messageID) {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        setPowerSave((bool)atoi(setPoint));
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
        setVS0(atof(setPoint));
        break;
    case 3:
        /* code */
        setVS1(atof(setPoint));
        break;
    case 4:
        /* code */
        setVFE(atof(setPoint));
        break;
    case 5:
        /* code */
        setVNO(atof(setPoint));
        break;
    case 6:
        /* code */
        setVNE(atof(setPoint));
        break;
    case 100:
        /* code */
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }

}

void MF_ASI::update()
{
    float pwmOutput = 0;
    // Do something which is required regulary
    if (ASIMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
    {
        lcd.fillScreen(TFT_BLACK);
        analogWrite(BACKLIGHT_PIN, 0);
    }
    else
    {

        pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
        analogWrite(BACKLIGHT_PIN, pwmOutput);
        drawGauge();
    }
    // lcd.setTextColor(TFT_GREEN);
    // // canvas.setTextFont(7);
    // lcd.setTextSize(1);
    // lcd.setCursor(100, 100);
    // lcd.println(String(pwmOutput));
}

void MF_ASI::drawGauge()
{
    RA_Airspeed.addValue(airSpeedFromSim);
    rawAngle = calculateAngle(RA_Airspeed.getAverage());

    RA_TASKnob.addValue(TASRatio);
    TASangle = scaleValue(RA_TASKnob.getAverage(), -1, 1, 15, -95);


    whiteArcStartAngle = calculateAngle(V_S1);
    whiteArcEndAngle = calculateAngle(V_FE);
    greenArcStartAngle = calculateAngle(V_S0);
    greenArcEndAngle = calculateAngle(V_NO);
    yellowArcStartAngle = greenArcEndAngle;
    yellowArcEndAngle = calculateAngle(V_NE);
    V_NEArcStartAngle = yellowArcEndAngle;
    V_NEArcEndAngle = V_NEArcStartAngle + 2;

    startTIme = millis();
    canvas.fillScreen(TFT_BLACK);

    drawLeftGauge();
    drawRightGauge();
}


void MF_ASI::drawLeftGauge()
{

    canvas.setPivot(240, 240);
    needleSpr.setPivot(ASI_NEEDLE_WIDTH / 2, 240);

    // Draw left half
    numberTapeSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);
    numberTapeSpr.setPivot(240, 240);
    numberTapeSpr.pushRotated(&canvas, TASangle, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);

    // Draw White Arc

    canvas.fillArc(240, 240, 215, 195, whiteArcStartAngle - 90, whiteArcEndAngle - 90, TFT_WHITE);

    // Draw Green Arc
    canvas.fillArc(240, 240, 195, 169, greenArcStartAngle - 90, greenArcEndAngle - 90, TFT_GREEN);

    // Draw Yellow Arc
    canvas.fillArc(240, 240, 195, 169, yellowArcStartAngle - 90, yellowArcEndAngle - 90, TFT_YELLOW);

    // Draw Red Arc for VNE
    canvas.fillArc(240, 240, 240, 169, V_NEArcStartAngle - 90, V_NEArcEndAngle - 90, TFT_RED);

    // Draw the labels
    labelsSpr.pushSprite(&canvas, 0, 0, BACKGROUND_COLOR);

    // Finally, draw the needle
    needleSpr.pushRotated(&canvas, rawAngle, BACKGROUND_COLOR);
    canvas.pushSprite(&lcd, 0, 0);
}

void MF_ASI::drawRightGauge()
{
    // Draw right half
    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240 - x_offset, 240);
    needleSpr.setPivot(ASI_NEEDLE_WIDTH / 2, 240);
    numberTapeSpr.pushRotated(&canvas, TASangle, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);

    // Draw White Arc
    canvas.fillArc(0, 240, 214, 195, whiteArcStartAngle - 90, whiteArcEndAngle - 90, TFT_WHITE);
    // Draw Green Arc
    canvas.fillArc(0, 240, 195, 169, greenArcStartAngle - 90, greenArcEndAngle - 90, TFT_GREEN);
    // Draw Yellow Arc
    canvas.fillArc(0, 240, 195, 169, yellowArcStartAngle - 90, yellowArcEndAngle - 90, TFT_YELLOW);
    // Draw Red Arc for VNE
    canvas.fillArc(0, 240, 240, 169, V_NEArcStartAngle - 90, V_NEArcEndAngle - 90, TFT_RED);
    // Draw the labels
    labelsSpr.pushSprite(&canvas, -x_offset, 0, BACKGROUND_COLOR);
    // Finally, draw the needle
    needleSpr.pushRotated(&canvas, rawAngle, BACKGROUND_COLOR);

    // Push the canvas sprite to the lcd screen
    canvas.pushSprite(&lcd, x_offset, 0);
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

// Setters
void MF_ASI::setAirSpeed(float value)
{
    airSpeedFromSim = value;
}

void MF_ASI::setTASRatio(float value)
{
    TASRatio = value;
}

void MF_ASI::setVS0(float value)
{
    V_S0 = value;
}


void MF_ASI::setVS1(float value)
{
    V_S1 = value;
}

void MF_ASI::setVFE(float value)
{
    V_FE = value;
}

void MF_ASI::setVNO(float value)
{
    V_NO = value;
}

void MF_ASI::setVNE(float value)
{
    V_NE = value;
}

void MF_ASI::setInstrumentBrightness(float value)
{
    float pwmOutput = 0;

    instrumentBrightness = scaleValue(value, 0, 1, 100, 255);
    pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // needed to correct PWM output due to human eye brightness perception
    analogWrite(BACKLIGHT_PIN, pwmOutput);
}

void MF_ASI::setPowerSave(bool enabled)
{
    powerSaveFlag = enabled;
}

// Scale Function
float MF_ASI::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
