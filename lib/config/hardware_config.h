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
  enum class HitDirection { Left, Right };

  HardwareConfig() {};
  HardwareConfig(JsonObject json);
  ~HardwareConfig() {};

  TurnType turnType() { return m_turnType; };
  String turnTypeString() { return turnTypeToString(m_turnType); };
  int frontPosition() { return m_frontPos; };
  int rearPosition() { return m_rearPos; };
  int singleSpeed() { return m_singleSpeed; }
  int infiniteSpeed() { return m_infiniteSpeed; }
  int maxHalfSpeed() { return m_maxHalfSpeed; }
  int maxFullSpeed() { return m_maxFullSpeed; }
  HitDirection hitDirection() { return m_hitDirection; };
  String hitDirectionString() { return hitDirectionToString(m_hitDirection); };

  void setTurnType(TurnType turnType);
  void setFrontPosition(int frontPos);
  void setRearPosition(int rearPos);
  void setSingleSpeed(int speed);
  void setInfiniteSpeed(int speed);
  void setMaxHalfSpeed(int value);
  void setMaxFullSpeed(int value);
  void setHitDirection(HitDirection hitDirection);

  void asJson(JsonObject &json);

private:
  TurnType m_turnType;
  int m_frontPos;
  int m_rearPos;
  int m_singleSpeed;
  int m_infiniteSpeed;
  int m_maxHalfSpeed;
  int m_maxFullSpeed;
  HitDirection m_hitDirection;

  TurnType turnTypeFromString(const String &turnTypeStr);
  String turnTypeToString(TurnType turnType) const;

  HitDirection hitDirectionFromString(const String &hitDirectionStr);
  String hitDirectionToString(HitDirection hitDirection) const;
};

#endif
