#include "LightAnim.h"
#include <FastLED.h>
#include <EEPROM.h>

LightAnim::LightAnim() {
}

LightAnim::~LightAnim() {}

void LightAnim::begin() {
  pinMode(RX_MODE_PIN, INPUT);
  pinMode(RX_BRIGHTNESS_PIN, INPUT);
  pinMode(RX_SPEED_PIN, INPUT);
  pinMode(RX_COLORCYCLE_PIN, INPUT);

  lastFrame = sizeof(noMAVLinkFrames) / sizeof(noMAVLinkFrames[0]) - 1;

  FastLED.addLeds<LED_TYPE, LIGHTS_1_PIN, COLOR_ORDER>(lights1LEDs, LIGHTS_1_LED_COUNT).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BASE_BRIGHTNESS );

  currentBlending = BLEND;
}

void LightAnim::loop() {

  // Occasionally check for signals from RX if it hasn't been seen
  if ( !rxModeActive && millis() > nextPWMTest ) {
    nextPWMTest = millis() + 5000;
    TestPWMActive();
  }

  InputTest();
  TestBattery();

  if ( MAVLINK_ENABLED && !MavLink_Connected ) {
    FastLED.setBrightness(  255 );
    RunFrameAnimation( noMAVLinkFrames, 1.0, noMAVLinkPalette );
    return;
  }
  else
  {
    if ( batteryLow )
    {
      if ( millis() > batteryAlertTimer ) {
        batteryAlertOn = !batteryAlertOn;
        modeChanged = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }

      if ( batteryAlertOn ) {
        FastLED.setBrightness(  255 );
        RunFrameAnimation( lowBatteryFrames, 1.0, noMAVLinkPalette );
        return;
      }
    }
  }

  if ( rxBrightnessInputActive && rxBrightnessInputRaw > 975 )
  {
    FastLED.setBrightness( map( constrain(rxBrightnessInputRaw, 1100, 1900), 1100, 1900, 0, 255 ) ); //max(0, min(255, (rxBrightnessInputRaw - 1100) / 3.0)) );
  }
  else
  {
    FastLED.setBrightness( BASE_BRIGHTNESS );
  }

  double speedModifier =  (double)map( constrain(rxSpeedInputRaw, 1100, 1900), 1100, 1900, 100, 8000 ) / 1000.0;

  switch ( currentMode ) {
    case MODE_MAVLINK_CONTROLLED:
      if ( ap_base_mode == 0 )
      {
        // Disarmed
        RunFrameAnimation( noMAVLinkFrames, 1.0, blackAndGreenPalette );
      }
      else
      {
        switch ( ap_custom_mode ) {
          case FLIGHTMODE_ACRO:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_ALTHOLD:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_AUTO:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_GUIDED:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_LOITER:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_RTL:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_CIRCLE:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_LAND:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_DRIFT:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_SPORT:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_FLIP:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_AUTOTUNE:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_POSHOLD:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          case FLIGHTMODE_BRAKE:
            RunFrameAnimation( solidColor, 2.5 );
            break;
          default:
            RunFrameAnimation( solidColor, 2.5 );
            break;
        }
      }
      break;
    case MODE_USER1:
      RunFrameAnimation( solidColor, speedModifier, userMode1Palettes[userMode1PaletteIndex] );
      break;
    case MODE_USER2:
      RunFrameAnimation( fadeUpAndDown, speedModifier, userMode2Palettes[userMode2PaletteIndex]  );
      break;
    case MODE_USER3:
      RunFrameAnimation( spacedChaser2, speedModifier, userMode3Palettes[userMode3PaletteIndex]  );
      break;
    case MODE_USER4:
      RunFrameAnimation( solidColor, speedModifier, userMode4Palettes[userMode4PaletteIndex]  );
      break;
    case MODE_USER5:
      RunFrameAnimation( spectrumScroller, speedModifier, userMode5Palettes[userMode5PaletteIndex]  );
      break;
  }
}

void LightAnim::UpdateMode(int newMode)
{
  if ( currentMode != newMode )
  {
    currentMode = newMode;
    modeChanged = true;
  }
}

void LightAnim::FindMode()
{
  if ( rxModeRaw < 975 )
  {
    if ( currentMode < 0 ) {
      UpdateMode( MODE_MAVLINK_CONTROLLED );
    }
  }
  else if ( rxModeRaw < MODE_1_THRESHOLD )
  {
    UpdateMode( MODE_MAVLINK_CONTROLLED );
  }
  else if ( rxModeRaw < MODE_2_THRESHOLD )
  {
    UpdateMode( MODE_USER1 );
  }
  else if ( rxModeRaw < MODE_3_THRESHOLD )
  {
    UpdateMode( MODE_USER2 );
  }
  else if ( rxModeRaw < MODE_4_THRESHOLD )
  {
    UpdateMode( MODE_USER3 );
  }
  else if ( rxModeRaw < MODE_5_THRESHOLD )
  {
    UpdateMode( MODE_USER4 );
  }
  else if ( rxModeRaw < MODE_6_THRESHOLD )
  {
    UpdateMode( MODE_USER5 );
  }
}

void LightAnim::TestPWMActive() {
  int testPWM = 0;
  testPWM = pulseIn(RX_MODE_PIN, HIGH, 20000);
  if ( testPWM > 0 ) {
    rxModeActive = true;
  }
  else
  {
    rxModeActive = false;
  }
  testPWM = pulseIn(RX_BRIGHTNESS_PIN, HIGH, 20000);
  if ( testPWM > 0 ) {
    rxBrightnessInputActive = true;
  }
  else
  {
    rxBrightnessInputActive = false;
  }
  testPWM = pulseIn(RX_SPEED_PIN, HIGH, 20000);
  if ( testPWM > 0 ) {
    rxSpeedInputActive = true;
  }
  else
  {
    rxSpeedInputActive = false;
  }
  testPWM = pulseIn(RX_COLORCYCLE_PIN, HIGH, 20000);
  if ( testPWM > 0 ) {
    rxColorCycleInputActive = true;
  }
  else
  {
    rxColorCycleInputActive = false;
  }

}

void LightAnim::InputTest() {
  if ( rxModeActive && millis() > nextModeInputTest ) {
    nextModeInputTest = millis() + 500;
    rxModeRaw = pulseIn(RX_MODE_PIN, HIGH, 20000);
    if (rxModeRaw == 0)
    {
      rxModeActive = false;
    }
    else
    {
      FindMode();
    }
  }

  if ( rxBrightnessInputActive && millis() > nextBrightnessInputTest ) {
    nextBrightnessInputTest = millis() + 500;
    rxBrightnessInputRaw = pulseIn(RX_BRIGHTNESS_PIN, HIGH, 20000);
    if (rxBrightnessInputRaw == 0)
    {
      rxBrightnessInputActive = false;
    }
  }

  if ( rxSpeedInputActive && millis() > nextSpeedInputTest ) {
    nextSpeedInputTest = millis() + 100;
    rxSpeedInputRaw = pulseIn(RX_SPEED_PIN, HIGH, 20000);
    if (rxSpeedInputRaw == 0)
    {
      rxSpeedInputActive = false;
    }
  }

  if ( rxColorCycleInputActive && millis() > nextColorCycleInputTest ) {
    nextColorCycleInputTest = millis() + 50;
    rxColorCycleInputRaw = pulseIn(RX_COLORCYCLE_PIN, HIGH, 20000);

    if (rxColorCycleInputRaw == 0)
    {
      rxColorCycleInputActive = false;
    }
    else
    {

      if ( rxColorCycleInputRaw > 1500 ) {
        if ( !paletteCycled )
        {
          paletteCycled = true;
          CycleCurrentPalette();
        }
      }
      else
      {
        paletteCycled = false;
      }
    }
  }
}

void LightAnim::TestBattery() {
  switch ( ap_cell_count ) {
    case 6:
      if ( ap_voltage_battery < 21000 )
      {
        batteryLow = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }
      else
      {
        batteryLow = false;
      }
      break;
    case 5:
      if ( ap_voltage_battery < 17500 )
      {
        batteryLow = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }
      else
      {
        batteryLow = false;
      }
      break;
    case 4:
      if ( ap_voltage_battery < 14000 )
      {
        batteryLow = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }
      else
      {
        batteryLow = false;
      }
      break;
    case 3:
      if ( ap_voltage_battery < 10500 )
      {
        batteryLow = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }
      else
      {
        batteryLow = false;
      }
      break;
    case 2:
      if ( ap_voltage_battery < 7000 )
      {
        batteryLow = true;
        batteryAlertTimer = millis() + BATT_ALERT_TIME;
      }
      else
      {
        batteryLow = false;
      }
      break;
  }
}

CRGBPalette16 LightAnim::GetCurrentPalette() {
  switch ( currentMode ) {
    case MODE_USER1:
      return userMode1Palettes[userMode1PaletteIndex];
      break;
    case MODE_USER2:
      return userMode2Palettes[userMode2PaletteIndex];
      break;
    case MODE_USER3:
      return userMode3Palettes[userMode3PaletteIndex];
      break;
    case MODE_USER4:
      return userMode4Palettes[userMode4PaletteIndex];
      break;
    case MODE_USER5:
      return userMode5Palettes[userMode5PaletteIndex];
      break;
  }

  return (CRGBPalette16)RainbowColors_p;
}

void LightAnim::CycleCurrentPalette() {
  switch ( currentMode ) {
    case MODE_USER1:
      userMode1PaletteIndex++;
      if ( userMode1PaletteIndex > sizeof(userMode1Palettes) / sizeof(CRGBPalette16) )
      {
        userMode1PaletteIndex = 0;
      }
      EEPROM.write(USERMODE1_PALETTE_INDEX_ADDRESS, userMode1PaletteIndex);
      break;
    case MODE_USER2:
      userMode2PaletteIndex++;
      if ( userMode2PaletteIndex > sizeof(userMode2Palettes) / sizeof(CRGBPalette16) )
      {
        userMode2PaletteIndex = 0;
      }
      EEPROM.write(USERMODE2_PALETTE_INDEX_ADDRESS, userMode2PaletteIndex);
      break;
    case MODE_USER3:
      userMode3PaletteIndex++;
      if ( userMode3PaletteIndex > sizeof(userMode3Palettes) / sizeof(CRGBPalette16) )
      {
        userMode3PaletteIndex = 0;
      }
      EEPROM.write(USERMODE3_PALETTE_INDEX_ADDRESS, userMode3PaletteIndex);
      break;
    case MODE_USER4:
      userMode4PaletteIndex++;
      if ( userMode4PaletteIndex > sizeof(userMode4Palettes) / sizeof(CRGBPalette16) )
      {
        userMode4PaletteIndex = 0;
      }
      EEPROM.write(USERMODE4_PALETTE_INDEX_ADDRESS, userMode4PaletteIndex);
      break;
    case MODE_USER5:
      userMode5PaletteIndex++;
      if ( userMode5PaletteIndex > sizeof(userMode5Palettes) / sizeof(CRGBPalette16) )
      {
        userMode5PaletteIndex = 0;
      }
      EEPROM.write(USERMODE5_PALETTE_INDEX_ADDRESS, userMode5PaletteIndex);
      break;
  }
}

