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
  int hitPosition() { return m_hitPos; };
  int restPosition() { return m_restPos; };
  int singleSpeed() { return m_singleSpeed; }
  int infiniteSpeed() { return m_infiniteSpeed; }
  int maxSingleSpeed() { return m_maxSingleSpeed; }
  int maxInfiniteSpeed() { return m_maxInfiniteSpeed; }
  HitDirection hitDirection() { return m_hitDirection; };
  String hitDirectionString() { return hitDirectionToString(m_hitDirection); };

  void setTurnType(TurnType turnType);
  void setHitPosition(int hitPos);
  void setRestPosition(int restPos);
  void setSingleSpeed(int speed);
  void setInfiniteSpeed(int speed);
  void setMaxSingleSpeed(int value);
  void setMaxInfiniteSpeed(int value);
  void setHitDirection(HitDirection hitDirection);

  void asJson(JsonObject &json);

private:
  TurnType m_turnType;
  int m_hitPos;
  int m_restPos;
  int m_singleSpeed;
  int m_infiniteSpeed;
  int m_maxSingleSpeed;
  int m_maxInfiniteSpeed;
  HitDirection m_hitDirection;

  TurnType turnTypeFromString(const String &turnTypeStr);
  String turnTypeToString(TurnType turnType) const;

  HitDirection hitDirectionFromString(const String &hitDirectionStr);
  String hitDirectionToString(HitDirection hitDirection) const;
};

#endif
