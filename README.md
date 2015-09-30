# teensyquadlights
Arduino code (Teensy 3.x) for running WS2812 lights on an RC craft

This is a fork of MavLink_FrSkySPort from this thread: http://diydrones.com/forum/topics/amp-to-frsky-x8r-sport-converter 

Very little of the original code has changed, other than a bit of integration with the main sketch file. The meat is in LightAnim.*

The additions to the original project include the following:
- 4 PWM (from rx) inputs used to control light settings (brightness, pattern, speed and color palette)
- Using telemetry data to optionally influence lighting (e.g. override lights with blinking red when batt is low)
- A fairly flexible animated light system built on FastLED library, allowing swapping palettes, arbitrary frame-based patterns for any number of lights, etc
- 4 discrete data line outputs for WS2812 light strings (though the source is a bit hard-wired right now for the first string)

A PCB has been created here: https://oshpark.com/projects/acMDnbtp to interface a Teensy with a SN74HCT245 logic shifter and protection resistors, and to break out the pins into headers for easy connections.
