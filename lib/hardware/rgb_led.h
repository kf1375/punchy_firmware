#include <Adafruit_NeoPixel.h>

class RGBLed
{
public:
  RGBLed();

  void begin();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void turnOff();

private:
  Adafruit_NeoPixel m_pexels;

  const uint8_t pin = 18;
  const uint8_t m_numPexels = 1; // How many WS2812B are attached?
};