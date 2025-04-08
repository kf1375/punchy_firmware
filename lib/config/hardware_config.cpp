#include "hardware_config.h"

#include "logging.h"

/**
 * @brief Construct a new HardwareConfig object from a JSON object.
 *
 * Initializes the HardwareConfig object using data from the provided JSON
 * object.
 *
 * @param json JSON object containing the hardware configuration parameters.
 */
HardwareConfig::HardwareConfig(JsonObject json)
{
  m_turnType = turnTypeFromString(json["turn_type"].as<String>());
  m_hitPos = json["hit_position"].as<int>();
  m_restPos = json["rest_position"].as<int>();
  m_singleSpeed = json["single_speed"].as<int>();
  m_infiniteSpeed = json["infinite_speed"].as<int>();
  m_maxHalfSpeed = json["max_half_speed"].as<int>();
  m_maxFullSpeed = json["max_full_speed"].as<int>();
  m_hitDirection = hitDirectionFromString(json["hit_direction"].as<String>());

  LOG_INFO("Hardware configuration loaded.");
}

/**
 * @brief Sets the turn type for the hardware configuration.
 *
 * This function updates the turn type value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param turnType The new turn type
 */
void HardwareConfig::setTurnType(TurnType turnType)
{
  if (m_turnType == turnType)
    m_changed = false;

  m_turnType = turnType;
  m_changed = true;
}

/**
 * @brief Sets the hitPos for the hardware configuration.
 *
 * This function updates the hitPos value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param hitPos The new hitPos
 */
void HardwareConfig::setHitPosition(int hitPos)
{
  m_changed = changeIntConfig(m_hitPos, hitPos);
}

/**
 * @brief Sets the restPos for the hardware configuration.
 *
 * This function updates the restPos value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param restPos The new restPos
 */
void HardwareConfig::setRestPosition(int restPos)
{
  m_changed = changeIntConfig(m_restPos, restPos);
}

/**
 * @brief Sets the single speed for the hardware configuration.
 *
 * This function updates the single speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param speed The new single speed
 */
void HardwareConfig::setSingleSpeed(int speed)
{
  m_changed = changeIntConfig(m_singleSpeed, speed);
}

/**
 * @brief Sets the infinite speed for the hardware configuration.
 *
 * This function updates the infinite speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param speed The new infinite speed
 */
void HardwareConfig::setInfiniteSpeed(int speed)
{
  m_changed = changeIntConfig(m_infiniteSpeed, speed);
}

/**
 * @brief Sets the max half speed for the hardware configuration.
 *
 * This function updates the max half speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param value The new max half speed
 */
void HardwareConfig::setMaxHalfSpeed(int value)
{
  m_changed = changeIntConfig(m_maxHalfSpeed, value);
}

/**
 * @brief Sets the max full speed for the hardware configuration.
 *
 * This function updates the max full speed value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param value The new max full speed
 */
void HardwareConfig::setMaxFullSpeed(int value)
{
  m_changed = changeIntConfig(m_maxHalfSpeed, value);
}

/**
 * @brief Sets the hit direction for the hardware configuration.
 *
 * This function updates the hit direction value and marks the configuration as
 * changed. It also sets the stored flag to true.
 *
 * @param hitDirection The new turn type
 */
void HardwareConfig::setHitDirection(HitDirection hitDirection)
{
  if (m_hitDirection == hitDirection)
    m_changed = false;

  m_hitDirection = hitDirection;
  m_changed = true;
}

/**
 * @brief Fill a given JSON object with the HardwareConfig data.
 *
 * This method serializes the safety configuration parameters into the provided
 * JSON object.
 *
 * @param json JSON object to populate with the hardware configuration
 * parameters.
 */
void HardwareConfig::asJson(JsonObject &json)
{
  json["turn_type"] = turnTypeToString(m_turnType);
  json["hit_position"] = m_hitPos;
  json["rest_position"] = m_restPos;
  json["single_speed"] = m_singleSpeed;
  json["infinite_speed"] = m_infiniteSpeed;
  json["max_half_speed"] = m_maxHalfSpeed;
  json["max_full_speed"] = m_maxFullSpeed;
  json["hit_direction"] = hitDirectionToString(m_hitDirection);
}

/**
 * @brief Convert a string to a TurnType enum.
 *
 * This method converts a string to a TurnType enum.
 *
 * @param turnTypeStr String representation of the TurnType.
 * @return HardwareController::TurnType The TurnType enum.
 */
HardwareConfig::TurnType
HardwareConfig::turnTypeFromString(const String &turnTypeStr)
{
  if (turnTypeStr == "HALF_TURN") {
    return TurnType::HalfTurn;
  } else if (turnTypeStr == "FULL_TURN") {
    return TurnType::FullTurn;
  } else {
    return TurnType::HalfTurn;
  }
}

/**
 * @brief Convert a TurnType enum to a string.
 *
 * This method converts a TurnType enum to a string.
 *
 * @param turnType The TurnType enum.
 * @return String The string representation of the TurnType.
 */
String HardwareConfig::turnTypeToString(TurnType turnType) const
{
  if (turnType == TurnType::HalfTurn) {
    return "HALF_TURN";
  } else if (turnType == TurnType::FullTurn) {
    return "FULL_TURN";
  } else {
    return "HALF_TURN";
  }
}

/**
 * @brief Convert a string to a hit direction enum.
 *
 * This method converts a string to a hit direction enum.
 *
 * @param hitDirectionStr String representation of the hit direction.
 * @return HardwareController::HitDirection The hit direction enum.
 */
HardwareConfig::HitDirection
HardwareConfig::hitDirectionFromString(const String &hitDirectionStr)
{
  if (hitDirectionStr == "LEFT") {
    return HitDirection::Left;
  } else if (hitDirectionStr == "RIGHT") {
    return HitDirection::Right;
  } else {
    return HitDirection::Left;
  }
}

/**
 * @brief Convert a HitDirection enum to a string.
 *
 * This method converts a HitDirection enum to a string.
 *
 * @param hitDirection The HitDirection enum.
 * @return String The string representation of the HitDirection.
 */
String HardwareConfig::hitDirectionToString(HitDirection hitDirection) const
{
  if (hitDirection == HitDirection::Left) {
    return "LEFT";
  } else if (hitDirection == HitDirection::Right) {
    return "RIGHT";
  } else {
    return "LEFT";
  }
}