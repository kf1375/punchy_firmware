
#include "abstract_config.h"

/**
 * @brief Check if the config changed
 *
 * Returns true if the config changed and also resets the changed flag
 *
 * @return true if the config changed
 */
bool AbstractConfig::changed()
{
  if (m_changed) {
    m_changed = false;
    return true;
  }
  return false;
}

/**
 * @brief change a float value in the config
 *
 * Changes the float value in the config if it changed significantly
 * will return true if it changed significantly
 *
 * @return true if it changed
 */
bool AbstractConfig::changeFloatConfig(float &old_config, float new_value)
{
  if (std::abs(old_config - new_value) < 1e-3)
    return false;

  m_changed = true;
  old_config = new_value;
  return true;
}
/**
 * @brief change a bool value in the config
 *
 * Changes the bool value in the config if it changed
 * will return true if it changed
 *
 * @return true if it changed
 */
bool AbstractConfig::changeBoolConfig(bool &old_config, bool new_value)
{
  if (old_config == new_value)
    return false;

  m_changed = true;
  old_config = new_value;
  return true;
}

/**
 * @brief change a String value in the config
 *
 * Changes the String value in the config if it changed
 * will return true if it changed
 *
 * @return true if it changed
 */
bool AbstractConfig::changeStringConfig(String &old_config, String new_value)
{
  if (old_config == new_value)
    return false;

  m_changed = true;
  old_config = new_value;
  return true;
}