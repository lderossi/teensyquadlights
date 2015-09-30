#ifndef LIGHTANIM_H
#define LIGHTANIM_H

#include <WProgram.h>
#include <FastLED.h>
#include <EEPROM.h>
#include "CustomPalettes.h"
#include "LightAnimFrames.h"


#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

#ifndef MAVLINK_ENABLED
#define MAVLINK_ENABLED 0
#endif

#define LIGHTS_1_PIN  20
#define LIGHTS_2_PIN  21
#define LIGHTS_3_PIN  22
#define LIGHTS_4_PIN  23

#define LIGHTS_1_LED_COUNT 8
#define LIGHTS_2_LED_COUNT 0
#define LIGHTS_3_LED_COUNT 0
#define LIGHTS_4_LED_COUNT 0

#define BASE_BRIGHTNESS 100

#define RX_MODE_PIN  3
#define RX_BRIGHTNESS_PIN  4
#define RX_SPEED_PIN  5
#define RX_COLORCYCLE_PIN  6

#define MODE_1_THRESHOLD 1200
#define MODE_2_THRESHOLD 1450
#define MODE_3_THRESHOLD 1550
#define MODE_4_THRESHOLD 1600
#define MODE_5_THRESHOLD 1700
#define MODE_6_THRESHOLD 2050

#define MODE_MAVLINK_CONTROLLED 1
#define MODE_USER1 2
#define MODE_USER2 3
#define MODE_USER3 4
#define MODE_USER4 5
#define MODE_USER5 6

#define BATT_ALERT_TIME 2000

#define FLIGHTMODE_STABILIZE 0
#define FLIGHTMODE_ACRO 1
#define FLIGHTMODE_ALTHOLD 2
#define FLIGHTMODE_AUTO 3
#define FLIGHTMODE_GUIDED 4
#define FLIGHTMODE_LOITER 5
#define FLIGHTMODE_RTL 6
#define FLIGHTMODE_CIRCLE 7
#define FLIGHTMODE_LAND 9
#define FLIGHTMODE_DRIFT 11
#define FLIGHTMODE_SPORT 13
#define FLIGHTMODE_FLIP 14
#define FLIGHTMODE_AUTOTUNE 15
#define FLIGHTMODE_POSHOLD 16
#define FLIGHTMODE_BRAKE 17

#define USERMODE1_PALETTE_INDEX_ADDRESS 0
#define USERMODE2_PALETTE_INDEX_ADDRESS 1
#define USERMODE3_PALETTE_INDEX_ADDRESS 2
#define USERMODE4_PALETTE_INDEX_ADDRESS 3
#define USERMODE5_PALETTE_INDEX_ADDRESS 4

class LightAnim {
  public:
    LightAnim();
    ~LightAnim();
    void begin();
    void loop();

  private:
    CRGB lights1LEDs[LIGHTS_1_LED_COUNT];
    CRGB lights2LEDs[LIGHTS_2_LED_COUNT];
    CRGB lights3LEDs[LIGHTS_3_LED_COUNT];
    CRGB lights4LEDs[LIGHTS_4_LED_COUNT];

    uint8_t userMode1PaletteIndex = EEPROM.read(USERMODE1_PALETTE_INDEX_ADDRESS);
    uint8_t userMode2PaletteIndex = EEPROM.read(USERMODE2_PALETTE_INDEX_ADDRESS);
    uint8_t userMode3PaletteIndex = EEPROM.read(USERMODE3_PALETTE_INDEX_ADDRESS);
    uint8_t userMode4PaletteIndex = EEPROM.read(USERMODE4_PALETTE_INDEX_ADDRESS);
    uint8_t userMode5PaletteIndex = EEPROM.read(USERMODE5_PALETTE_INDEX_ADDRESS);

    const CRGBPalette16 userMode1Palettes[2] = { blackAndRedPalette, blackAndGreenPalette };
    const CRGBPalette16 userMode2Palettes[8] = { blackAndRedPalette, blackAndOrangePalette, blackAndYellowPalette, blackAndGreenPalette, blackAndBluePalette,
                                                 blackAndIndigoPalette, blackAndVioletPalette, blackAndWhitePalette
                                               };
    const CRGBPalette16 userMode3Palettes[8] = { blackAndRedPalette, blackAndOrangePalette, blackAndYellowPalette, blackAndGreenPalette, blackAndBluePalette,
                                                 blackAndIndigoPalette, blackAndVioletPalette, blackAndWhitePalette
                                               };
    const CRGBPalette16 userMode4Palettes[8] = { blackAndRedPalette, blackAndOrangePalette, blackAndYellowPalette, blackAndGreenPalette, blackAndBluePalette,
                                                 blackAndIndigoPalette, blackAndVioletPalette, blackAndWhitePalette
                                               };
    const CRGBPalette16 userMode5Palettes[3] = { (CRGBPalette16)RainbowStripeColors_p, (CRGBPalette16)OceanColors_p, (CRGBPalette16)PartyColors_p };

    TBlendType    currentBlending;

    bool paletteCycled = false;

    uint8_t lastFrame = 0;
    uint8_t currentFrame = 0;
    uint32_t frameStartTime = 0;

    uint32_t nextModeInputTest = 0;
    uint32_t nextBrightnessInputTest = 0;
    uint32_t nextSpeedInputTest = 0;
    uint32_t nextColorCycleInputTest = 0;
    uint32_t nextPWMTest = 0;

    uint8_t currentMode = 1;
    bool modeChanged = true;

    uint16_t rxModeRaw = 0;
    uint16_t rxBrightnessInputRaw = 0;
    uint16_t rxSpeedInputRaw = 0;
    uint16_t rxColorCycleInputRaw = 0;

    bool rxModeActive = 0;
    bool rxBrightnessInputActive = 0;
    bool rxSpeedInputActive = 0;
    bool rxColorCycleInputActive = 0;

    bool batteryLow = false;
    bool batteryAlertOn = false;
    uint32_t batteryAlertTimer = 0;

    void UpdateMode(int);
    void FindMode();
    void TestPWMActive();
    void TestBattery();
    void InputTest();
    CRGBPalette16 GetCurrentPalette();
    void CycleCurrentPalette();

    template< typename T, size_t N, size_t X >
    void RunFrameAnimation( T (&frames)[N][X], float speedMultiplier ) {
      RunFrameAnimation( frames, speedMultiplier, GetCurrentPalette() );
    }

    template< typename T, size_t N, size_t X >
    void RunFrameAnimation( T (&frames)[N][X], double speedMultiplier, const struct CRGBPalette16& palette ) {
      if ( modeChanged )
      {
        // When frameset changes, reset frame indices
        modeChanged = false;
        currentFrame = sizeof(frames) / sizeof(frames[0]) - 1;
        frameStartTime = 0;
      }

      CRGBPalette16 localPalette = palette;
      if ( localPalette == NULL )
      {
        localPalette = GetCurrentPalette();
      }

      if ( millis() >= frameStartTime + (frames[currentFrame][0] / speedMultiplier) + (frames[currentFrame][1] / speedMultiplier) ) {
        lastFrame = currentFrame;
        currentFrame++;
        if ( currentFrame >= sizeof(frames) / sizeof(frames[0]) )
        {
          currentFrame = 0;
        }

        frameStartTime = millis();
      }

      if ( millis() < frameStartTime + (frames[currentFrame][0] / speedMultiplier) ) {
        // Interplate
        float timeDelta = (frames[currentFrame][0] / speedMultiplier) - (frameStartTime + (frames[currentFrame][0]  / speedMultiplier) - millis());
        float interpFactor = 1 - (timeDelta / float(frames[lastFrame][0]  / speedMultiplier));
        uint8_t pixelIndex = 2;

        for (uint8_t i = 0; i < LIGHTS_1_LED_COUNT; i++) {
          CRGB currentColor = ColorFromPalette( palette, frames[currentFrame][pixelIndex], 255, currentBlending);
          CRGB lastColor = ColorFromPalette( palette, frames[lastFrame][pixelIndex], 255, currentBlending);

          uint8_t currentRed = currentColor.r; //colorTable[frames[currentFrame][pixelIndex]][0];
          uint8_t currentGreen = currentColor.g; //colorTable[frames[currentFrame][pixelIndex]][1];
          uint8_t currentBlue = currentColor.b; //colorTable[frames[currentFrame][pixelIndex]][2];
          uint8_t redInterp = currentRed + ((lastColor.r - currentRed) * interpFactor);
          uint8_t greenInterp = currentGreen + ((lastColor.g - currentGreen) * interpFactor);
          uint8_t blueInterp = currentBlue + ((lastColor.b - currentBlue) * interpFactor);

          lights1LEDs[i] = CRGB(redInterp, greenInterp, blueInterp);

          pixelIndex++;
          if ( pixelIndex >= sizeof( frames[currentFrame] ) / sizeof( uint16_t) )
          {
            pixelIndex = 2;
          }
        }

        FastLED.show();
      }
      else if ( millis() < frameStartTime + (frames[currentFrame][0]  / speedMultiplier) + (frames[currentFrame][1] / speedMultiplier) )
      {
        // Hold time; set the pixels to the new colors 100% and wait
        uint8_t pixelIndex = 2;

        for (uint8_t i = 0; i < LIGHTS_1_LED_COUNT; i++) {
          lights1LEDs[i] = ColorFromPalette( palette, frames[currentFrame][pixelIndex], 255, currentBlending);
          pixelIndex++;
          if ( pixelIndex >= sizeof( frames[currentFrame] ) / sizeof( uint16_t) )
          {
            pixelIndex = 2;
          }
        }

        FastLED.show();
      }
    }
};

#endif
