#ifndef RGB_LED_H
#define RGB_LED_H

#include <FastLED.h>

class RGBLed
{
public:
  RGBLed();
  void begin();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void turnOff();

private:
  static constexpr uint8_t pin = 18;
  static constexpr uint8_t numLeds = 1;
  CRGB leds[numLeds];
};

#endif // RGB_LED_H
