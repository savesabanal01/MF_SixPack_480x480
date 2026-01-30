#include "MF_VOR.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "4inchLCDConfig_Guition.h"
#include "Common_Bezel.h" // present in project, but bezel rendering removed per request
#include "RunningAverage.h"
#include "LCDBrightnessTable.h"
#include "runningAngle.h"

#define BACKGROUND_COLOR  0x1041
#define NAV_COLOR         TFT_GREEN
#define NAV_CENTER_X      240
#define NAV_CENTER_Y      240

// How many pixels per "dot" of deviation to move the indicators.
static constexpr float NAV_PX_PER_DOT = 15.0f;
// Typical max deflection is ~2.5 dots; we clamp to keep things on the face.
static constexpr float NAV_MAX_DOT    = 2.5f;

// Indicator sizes (in pixels)
static constexpr int NAV_CDI_HALF_HEIGHT = 32;   // vertical bar half height
static constexpr int NAV_CDI_WIDTH       = 5;    // vertical bar thickness
static constexpr int NAV_GS_HALF_WIDTH   = 32;   // horizontal bar half width
static constexpr int NAV_GS_HEIGHT       = 5;    // horizontal bar thickness
static constexpr int NAV_CENTER_BOX      = 10;   // small center box size

// Flag styling / placement
static constexpr uint16_t FLAG_NAV_BG   = TFT_RED;
static constexpr uint16_t FLAG_TOFR_BG  = TFT_YELLOW;
static constexpr uint16_t FLAG_TEXT     = TFT_BLACK;

static constexpr int FLAG_NAV_X         = 14;   // px from left edge of each half
static constexpr int FLAG_NAV_Y         = 14;   // px from top
static constexpr int FLAG_NAV_W         = 60;
static constexpr int FLAG_NAV_H         = 26;

static constexpr int FLAG_TOFR_X        = 150;  // px from left edge of each half
static constexpr int FLAG_TOFR_Y        = 392;  // near bottom
static constexpr int FLAG_TOFR_W        = 70;
static constexpr int FLAG_TOFR_H        = 26;

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite headingTapeSpr(&canvas);
static LGFX_Sprite mainGaugeSpr(&canvas);

runningAngle RA_Heading(runningAngle::DEGREES);

int HIMessageID = -1;

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

MF_VOR::MF_VOR(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void MF_VOR::begin()
{
}

void MF_VOR::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
    lcd.init();
    lcd.setFont(&fonts::Font4);

    lcd.setRotation(3);

    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    delay(1000);

    // Each half is rendered on a 240x480 canvas and pushed to the 480x480 LCD
    canvas.createSprite(240, 480);

    mainGaugeSpr.setBuffer(const_cast<std::uint16_t *>(HI_Main_Gauge), HI_MAIN_GAUGE_WIDTH, HI_MAIN_GAUGE_HEIGHT, 16);
    headingTapeSpr.setBuffer(const_cast<std::uint16_t *>(HI_Heading_Tape), HI_HEADING_TAPE_WIDTH, HI_HEADING_TAPE_HEIGHT, 16);
    // Bezel intentionally not used (user requested no bezel/frame)

    RA_Heading.reset();
}

void MF_VOR::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
    canvas.deleteSprite();
    mainGaugeSpr.deleteSprite();
    headingTapeSpr.deleteSprite();
    lcd.endWrite();
}

void MF_VOR::set(int16_t messageID, char *setPoint)
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

    HIMessageID = messageID;

    switch (messageID) {
    case -1:
        // called when Mobiflight shuts down
        break;
    case -2:
        // called when PowerSavingMode is entered
        setPowerSave((bool)atoi(setPoint));
        break;
    case 0:
        // Heading
        setHeading(atof(setPoint));
        break;
    case 1:
        // VOR / CDI deviation (Mobiflight variable name: VORind)
        setVORind(atof(setPoint));
        break;
    case 2:
        // Glideslope deviation (Mobiflight variable name: GSind)
        setGSind(atof(setPoint));
        break;
    case 3:
        // TO/FROM flag (Mobiflight variable name: TOFROMind)
        // Expected: -1 = FROM, 0 = OFF, +1 = TO
        setTOFROMind(atof(setPoint));
        break;
    case 4:
        // NAV flag (Mobiflight variable name: NAVflag)
        // Expected: 1 = show NAV flag (invalid), 0 = hide
        setNAVflag(atof(setPoint));
        break;
    case 100:
        // Instrument brightness (0..1)
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }
}

void MF_VOR::update()
{
    RA_Heading.add(heading);

    headingAverage = RA_Heading.getAverage();
    if (headingAverage < 0)
        headingAverage += 360;

    if (HIMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
    {
        lcd.fillScreen(TFT_BLACK);
        canvas.fillSprite(TFT_BLACK);
        analogWrite(BACKLIGHT_PIN, 0);
    }
    else
    {
        float pwmOutput = 0;
        pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // perceptual correction
        analogWrite(BACKLIGHT_PIN, pwmOutput);
        drawGauge();
    }
}

void MF_VOR::drawGauge()
{
    canvas.fillScreen(TFT_BLACK);

    drawLeftGauge();
    drawRightGauge();
}

void MF_VOR::drawNavIndicators(int16_t x_shift)
{
    // Convert NAV deviations (dots) to pixels.
    float cdiDots = clampf(VORind, -NAV_MAX_DOT, NAV_MAX_DOT);
    float gsDots  = clampf(GSind,  -NAV_MAX_DOT, NAV_MAX_DOT);

    int cdiX = (int)(NAV_CENTER_X + x_shift + (cdiDots * NAV_PX_PER_DOT));
    int gsY  = (int)(NAV_CENTER_Y - (gsDots * NAV_PX_PER_DOT));

    // CDI: vertical bar that moves left/right
    canvas.fillRect(cdiX - (NAV_CDI_WIDTH / 2),
                    NAV_CENTER_Y - NAV_CDI_HALF_HEIGHT,
                    NAV_CDI_WIDTH,
                    NAV_CDI_HALF_HEIGHT * 2,
                    NAV_COLOR);

    // Glideslope: horizontal bar that moves up/down
    canvas.fillRect((NAV_CENTER_X + x_shift) - NAV_GS_HALF_WIDTH,
                    gsY - (NAV_GS_HEIGHT / 2),
                    NAV_GS_HALF_WIDTH * 2,
                    NAV_GS_HEIGHT,
                    NAV_COLOR);

    // Small center box (helps visually, and provides "center" reference)
    canvas.drawRect((NAV_CENTER_X + x_shift) - (NAV_CENTER_BOX / 2),
                    NAV_CENTER_Y - (NAV_CENTER_BOX / 2),
                    NAV_CENTER_BOX,
                    NAV_CENTER_BOX,
                    NAV_COLOR);

    // --- Flags ---
    // x_shift is 0 for the left half and -240 for the right half.
    const int halfOriginGlobalX = (x_shift == 0) ? 0 : 240;

    // NAV flag: shown when nav signal is invalid.
    if (NAVflag >= 0.5f)
    {
        const int fx = (halfOriginGlobalX + FLAG_NAV_X) + x_shift;
        const int fy = FLAG_NAV_Y;
        canvas.fillRect(fx, fy, FLAG_NAV_W, FLAG_NAV_H, FLAG_NAV_BG);
        canvas.setTextColor(FLAG_TEXT, FLAG_NAV_BG);
        canvas.setTextDatum(middle_center);
        canvas.drawString("NAV", fx + (FLAG_NAV_W / 2), fy + (FLAG_NAV_H / 2));
    }

    // TO/FROM flag: -1=FROM, +1=TO, 0=hidden
    if (TOFROMind > 0.5f || TOFROMind < -0.5f)
    {
        const int fx = (halfOriginGlobalX + FLAG_TOFR_X) + x_shift;
        const int fy = FLAG_TOFR_Y;
        canvas.fillRect(fx, fy, FLAG_TOFR_W, FLAG_TOFR_H, FLAG_TOFR_BG);
        canvas.setTextColor(FLAG_TEXT, FLAG_TOFR_BG);
        canvas.setTextDatum(middle_center);
        if (TOFROMind > 0.5f)
            canvas.drawString("TO", fx + (FLAG_TOFR_W / 2), fy + (FLAG_TOFR_H / 2));
        else
            canvas.drawString("FR", fx + (FLAG_TOFR_W / 2), fy + (FLAG_TOFR_H / 2));
    }
}

void MF_VOR::drawLeftGauge()
{
    // Draw left half (global x: 0..239)
    canvas.fillScreen(TFT_BLACK);
    canvas.setPivot(240, 240);
    headingTapeSpr.setPivot(240, 240);
    headingTapeSpr.pushRotated(&canvas, -headingAverage, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, 90, 90, BACKGROUND_COLOR);

    // NAV indicators overlay
    drawNavIndicators(0);
    canvas.pushSprite(&lcd, 0, 0);
}

void MF_VOR::drawRightGauge()
{
    // Draw right half (global x: 240..479)
    canvas.fillScreen(TFT_BLACK);
    headingTapeSpr.setPivot(240, 240);
    canvas.setPivot(240 - x_offset, 240);
    headingTapeSpr.pushRotated(&canvas, -headingAverage, BACKGROUND_COLOR);
    mainGaugeSpr.pushSprite(&canvas, 90 - x_offset, 90, BACKGROUND_COLOR);

    // NAV indicators overlay (shift x by -240 so global coords map into right-half canvas)
    drawNavIndicators(-x_offset);
    canvas.pushSprite(&lcd, x_offset, 0);
}

// Setters
void MF_VOR::setHeading(float value)
{
    heading = value;
}

void MF_VOR::setVORind(float value)
{
    VORind = value;
}

void MF_VOR::setGSind(float value)
{
    GSind = value;
}

void MF_VOR::setTOFROMind(float value)
{
    // Normalize to -1 / 0 / +1
    if (value > 0.5f) TOFROMind = 1.0f;
    else if (value < -0.5f) TOFROMind = -1.0f;
    else TOFROMind = 0.0f;
}

void MF_VOR::setNAVflag(float value)
{
    NAVflag = (value >= 0.5f) ? 1.0f : 0.0f;
}

void MF_VOR::setPowerSave(bool enabled)
{
    powerSaveFlag = enabled;
}

void MF_VOR::setInstrumentBrightness(float value)
{
    float pwmOutput = 0;

    instrumentBrightness = scaleValue(value, 0, 1, 100, 255);
    pwmOutput = CIE_LIGHTNESS_TO_PWM_LUT_256_IN_8BIT_OUT[(int)instrumentBrightness]; // perceptual correction
    analogWrite(BACKLIGHT_PIN, pwmOutput);
}

// Utility
float MF_VOR::clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Scale Function
float MF_VOR::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
