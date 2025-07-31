#include "rgb_led.h"

RGBLed::RGBLed() {}

void RGBLed::begin()
{
  FastLED.addLeds<WS2812B, pin, GRB>(leds, numLeds);
}

void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  leds[0] = CRGB(r, g, b);
  FastLED.show();
}

void RGBLed::turnOff()
{
  leds[0] = CRGB::Black;
  FastLED.show();
}
