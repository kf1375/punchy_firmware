#ifndef SPEED_CONFIG_H
#define SPEED_CONFIG_H

#include "Arduino.h"
#include "ArduinoJson.h"

#include "abstract_config.h"

//************************************************************************************************
// Config Class for various hardware relevant variables
// Provides getters and setters for those
// ***********************************************************************************************
class HardwareConfig : public AbstractConfig
{
public:
  enum class TurnType { HalfTurn, FullTurn };

  HardwareConfig() {};
  HardwareConfig(JsonObject json);
  ~HardwareConfig() {};

  TurnType turnType() { return m_turnType; };
  String turnTypeString() { return turnTypeToString(m_turnType); };
  int frontPosition() { return m_frontPos; };
  int singleSpeed() { return m_singleSpeed; }
  int infiniteSpeed() { return m_infiniteSpeed; }
  int maxHalfSpeed() { return m_maxHalfSpeed; }
  int maxFullSpeed() { return m_maxFullSpeed; }

  void setTurnType(TurnType turnType) { m_turnType = turnType; };
  void setFrontPosition(int frontPos) { m_frontPos = frontPos; };
  void setSingleSpeed(int speed) { m_singleSpeed = speed; };
  void setInfiniteSpeed(int speed) { m_infiniteSpeed = speed; };
  void setMaxHalfSpeed(int value) { m_maxHalfSpeed = value; };
  void setMaxFullSpeed(int value) { m_maxFullSpeed = value; };

  void asJson(JsonObject &json);

private:
  TurnType m_turnType;
  int m_frontPos;
  int m_singleSpeed;
  int m_infiniteSpeed;
  int m_maxHalfSpeed;
  int m_maxFullSpeed;

  TurnType turnTypeFromString(const String &turnTypeStr);
  String turnTypeToString(TurnType turnType) const;
};

#endif
