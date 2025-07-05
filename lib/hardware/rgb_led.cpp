#include "rgb_led.h"

// Constructor
RGBLed::RGBLed() : m_pexels(m_numPexels, pin, NEO_GRB + NEO_KHZ800) {}

// Initialize the LED
void RGBLed::begin()
{
  m_pexels.begin();
}

// Set color
void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  m_pexels.setPixelColor(0, m_pexels.Color(r, g, b));
  m_pexels.show();
}

// Turn off the LED
void RGBLed::turnOff()
{
  m_pexels.clear();
}