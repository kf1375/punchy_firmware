#include "mqtt_client.h"

#include <vector>

#include "logging.h"
#include "util.h"

/**
 * @brief Construct a new MqttClient object
 *
 * @param mgr Mongoose event manager
 */
MqttClient::MqttClient(mg_mgr &mgr, Configuration &config,
                       HardwareController &hwController)
    : m_mgr(mgr), m_config(config), m_hwController(hwController)
{
}

/**
 * @brief Initializes the MQTT client by setting connection options,
 * credentials, and starting an MQTT listener.
 */
void MqttClient::setup()
{
  m_stopped = false;
  m_mqttPrefix = Util::getMacAddress();
  LOG_INFO("Setting up new MqttClient instance with prefix " + m_mqttPrefix);

  // Copy credentials locally
  String userString = m_config.mqtt.getUser();
  String passString = m_config.mqtt.getPass();
  String willString = m_mqttPrefix + "/will";
  String clientIDString = m_mqttPrefix + "_client";

  // MQTT connection options
  struct mg_mqtt_opts opts = {
      .user = mg_str(userString.c_str()),
      .pass = mg_str(passString.c_str()),
      .client_id = mg_str(clientIDString.c_str()),
      .topic = mg_str(willString.c_str()),
      .message = mg_str("bye"),
      .qos = 1,
      .version = 4,
      .keepalive = 60,
      .clean = true,
  };

  // Establish MQTT connection
  String brokerUrl = m_config.mqtt.getAddress();
  m_mqttConn = mg_mqtt_connect(
      &m_mgr, brokerUrl.c_str(), &opts,
      [](mg_connection *c, int ev, void *ev_data) {
        static_cast<MqttClient *>(c->fn_data)->eventHandler(c, ev, ev_data);
      },
      this);
  LOG_INFO("Starting MQTT listener on " + brokerUrl);
}

/**
 * @brief Stops the MQTT client, disconnecting any active connections.
 */
void MqttClient::stop()
{
  if (m_mqttConn != nullptr) {
    LOG_INFO("Stopping MQTT listener")
    struct mg_mqtt_opts *opts;
    mg_mqtt_disconnect(m_mqttConn, opts);
    m_mqttConn->is_draining = true;
  }
  m_stopped = true;
}

/**
 * @brief Restarts the MQTT client if it is not already stopped.
 */
void MqttClient::restart()
{
  if (!m_stopped)
    setup();
}

/**
 * @brief Handles the main loop for the MQTT client, including discovery,
 * subscription, and publishing messages.
 */
void MqttClient::run()
{
  if (isClientConnected()) {
    if (m_discoveryIsNecessary) {
      m_discoveryIsNecessary = false;
      subscribe();
      m_lastPublishTimestamp_ms = millis();
    }
    // Publish data if delay has elapsed and buffer is not full
    // if (Util::timeElapsed(1, m_lastPublishTimestamp_ms) &&
    // (mqtt_conn->send.len < 1000)) {
    //     LOG_DEBUG("MQTT buffer length before send: " +
    //     String(mqtt_conn->send.len)); m_lastPublishTimestamp_ms = millis();
    //     publish();
    // }
  } else {
    m_discoveryIsNecessary = true;
  }
}

/**
 * @brief Subscribes to predefined MQTT topics for controlling the device.
 */
void MqttClient::subscribe()
{
  LOG_INFO("Subscribing to MQTT topics with prefix " + m_mqttPrefix);
  struct mg_mqtt_opts sub_opts;
  memset(&sub_opts, 0, sizeof(sub_opts));

  // Define topics to subscribe to
  std::vector<String> topics = {m_mqttPrefix + "/pair",
                                m_mqttPrefix + "/settings/turn_type",
                                m_mqttPrefix + "/status",
                                m_mqttPrefix + "/settings/set_front",
                                m_mqttPrefix + "/unpair",
                                m_mqttPrefix + "/settings/set_rear",
                                m_mqttPrefix + "/start/single",
                                m_mqttPrefix + "/settings/max_half_speed",
                                m_mqttPrefix + "/start/infinite",
                                m_mqttPrefix + "/settings/max_full_speed",
                                m_mqttPrefix + "/stop",
                                m_mqttPrefix + "/commands/up",
                                m_mqttPrefix + "/commands/down",
                                m_mqttPrefix + "/commands/update"};

  // Subscribe to each topic
  for (auto topic : topics) {
    sub_opts.topic = mg_str(topic.c_str());
    mg_mqtt_sub(m_mqttConn, &sub_opts);
  }
}

/**
 * @brief do the cleaning stuff when the mqtt is closed and restart the
 * connection
 *
 */
void MqttClient::close()
{
  m_failCount++;
  LOG_INFO("MQTT CLOSED fail count " + String(m_failCount));
  m_mqttConn = nullptr; // Mark the connection as closed
  m_connected = false;

  // Stop the device

  // Reconnect if the fail count is within limits
  if (m_failCount < 5 && !m_stopped) {
    LOG_INFO("Reconnecting in 5 seconds");
    mg_timer_add(
        &m_mgr, 5000, 0,
        [](void *arg) { static_cast<MqttClient *>(arg)->restart(); }, this);
  }
}

/**
 * @brief Publishes data to the specified MQTT topic with an optional retain
 * flag.
 * @param topic The MQTT topic to publish to.
 * @param data The message payload.
 * @param retain Flag indicating whether the message should be retained.
 */
void MqttClient::publishData(String topic, String data, bool retain)
{
  if (!m_mqttConn)
    return;

  struct mg_mqtt_opts pub_opts;
  memset(&pub_opts, 0, sizeof(pub_opts));
  pub_opts.qos = 0;
  pub_opts.topic = mg_str(topic.c_str());
  pub_opts.message = mg_str(data.c_str());
  pub_opts.retain = retain;

  // Send the MQTT message
  mg_mqtt_pub(m_mqttConn, &pub_opts);
}

/**
 * @brief Checks if the MQTT client is currently connected.
 * @return True if connected, false otherwise.
 */
bool MqttClient::isClientConnected()
{
  return m_mqttConn != nullptr && m_failCount == 0 && m_connected;
}

/**
 * @brief Handle incoming messages for subscribed topics
 *
 * @param c The MQTT connection.
 * @param topic The MQTT topic to publish to.
 * @param data The message payload.
 */
void MqttClient::onMessageReceived(struct mg_connection *c, const String &topic,
                                   const String &data)
{
  if (topic == m_mqttPrefix + "/pair") {
    handlePair(c, data);
  } else if (topic == m_mqttPrefix + "/unpair") {
    handleUnpair(c, data);
  } else if (topic == m_mqttPrefix + "/status") {
    handleStatus(c, data);
  } else if (topic == m_mqttPrefix + "/start/single") {
    handleStartSingle(c, data);
  } else if (topic == m_mqttPrefix + "/start/infinite") {
    handleStartInfinite(c, data);
  } else if (topic == m_mqttPrefix + "/stop") {
    handleStop(c, data);
  } else if (topic == m_mqttPrefix + "/commands/up") {
    handleCommandUp(c, data);
  } else if (topic == m_mqttPrefix + "/commands/down") {
    handleCommandDown(c, data);
  } else if (topic == m_mqttPrefix + "/commands/update") {
    handleCommandUpdate(c, data);
  } else if (topic == m_mqttPrefix + "/settings/turn_type") {
    handleSettingTurnType(c, data);
  } else if (topic == m_mqttPrefix + "/settings/set_front") {
    handleSettingFrontPos(c, data);
  } else if (topic == m_mqttPrefix + "/settings/set_rear") {
    handleSettingRearPos(c, data);
  } else if (topic == m_mqttPrefix + "/settings/max_half_speed") {
    handleSettingMaxHalfSpeed(c, data);
  } else if (topic == m_mqttPrefix + "/settings/max_full_speed") {
    handleSettingMaxFullSpeed(c, data);
  }
}

/**
 * @brief Handle the incoming message on /pair topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handlePair(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Pair");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  if (doc["type"].as<String>() == "request") {
    publishData(m_mqttPrefix + "/pair",
                "{\"type\":\"response\",\"status\":\"accepted\","
                "\"message\":\"Device paired successfully\"}");
    LOG_INFO("Pairing response published successfully");
  }
}

/**
 * @brief Handle the incoming messages on /status topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleStatus(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Status");
}

/**
 * @brief Handle the incoming message on /unpair topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleUnpair(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Unpair");
}

/**
 * @brief Handle the incoming message on /start/single topic
 *
 * @param c The MQTT connection
 * @param data The message payload { speed: (value) }
 */
void MqttClient::handleStartSingle(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Start Single");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  int singleSpeed = doc["speed"].as<int>();
  m_config.hardware.setSingleSpeed(singleSpeed);

  m_hwController.setNextState(HardwareController::State::SingleTurn);
}

/**
 * @brief Handle the incoming message on /start/infinite topic
 *
 * @param c The MQTT connection
 * @param data The message payload { speed: (value) }
 */
void MqttClient::handleStartInfinite(struct mg_connection *c,
                                     const String &data)
{
  LOG_INFO("Handle Start Infinite");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  int infiniteSpeed = doc["speed"].as<int>();
  m_config.hardware.setInfiniteSpeed(infiniteSpeed);

  m_hwController.setNextState(HardwareController::State::InfiniteTurn);
}

/**
 * @brief Handle the incoming message on /stop topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleStop(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Start Stop");
  m_hwController.setNextState(HardwareController::State::Stop);
}

/**
 * @brief Handle the incoming message on /settings/turn_type topic
 *
 * @param c The MQTT connection
 * @param data The message payload { value: (turn_type) }
 */
void MqttClient::handleSettingTurnType(struct mg_connection *c,
                                       const String &data)
{
  LOG_INFO("Handle Set Turn Type");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  HardwareConfig::TurnType turnType;
  String turnTypeStr = doc["value"].as<String>();
  if (turnTypeStr == "Half Turn")
    turnType = HardwareConfig::TurnType::HalfTurn;
  else if (turnTypeStr == "Full Turn")
    turnType = HardwareConfig::TurnType::FullTurn;
  else
    turnType = HardwareConfig::TurnType::HalfTurn;

  m_config.hardware.setTurnType(turnType);
}

/**
 * @brief Handle the incoming message on /settings/set_front topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleSettingFrontPos(struct mg_connection *c,
                                       const String &data)
{
  LOG_INFO("Handle Set Front Pos");

  m_hwController.setFrontPos();
}

/**
 * @brief Handle the incoming message on /settings/set_rear topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleSettingRearPos(struct mg_connection *c,
                                      const String &data)
{
  LOG_INFO("Handle Set Rear Pos");

  m_hwController.setRearPos();
}

/**
 * @brief Handle the incoming message on /settings/max_half_speed topic
 *
 * @param c The MQTT connection
 * @param data The message payload { value: (speed [RPM]) }
 */
void MqttClient::handleSettingMaxHalfSpeed(struct mg_connection *c,
                                           const String &data)
{
  LOG_INFO("Handle Set Max Half Speed");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  int maxHalfSpeed = doc["value"].as<int>();
  m_config.hardware.setMaxHalfSpeed(maxHalfSpeed);
}

/**
 * @brief Handle the incoming message on /settings/max_full_speed topic
 *
 * @param c The MQTT connection
 * @param data The message payload { value: (speed [RPM]) }
 */
void MqttClient::handleSettingMaxFullSpeed(struct mg_connection *c,
                                           const String &data)
{
  LOG_INFO("Handle Set Max Full Speed");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    LOG_INFO("Failed to parse JSON");
    return;
  }

  int maxFullSpeed = doc["value"].as<int>();
  m_config.hardware.setMaxFullSpeed(maxFullSpeed);
}

/**
 * @brief Handle the incoming message on /commands/up topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleCommandUp(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Command Up");

  m_hwController.setManualCommand(HardwareController::ManualCommand::Forward);
  m_hwController.setNextState(HardwareController::State::ManualTurn);
}

/**
 * @brief Handle the incoming message on /commands/down topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleCommandDown(struct mg_connection *c, const String &data)
{
  LOG_INFO("Handle Command Down");

  m_hwController.setManualCommand(HardwareController::ManualCommand::Backward);
  m_hwController.setNextState(HardwareController::State::ManualTurn);
}

/**
 * @brief Handle the incoming message on /commands/update topic
 *
 * @param c The MQTT connection
 * @param data The message payload
 */
void MqttClient::handleCommandUpdate(struct mg_connection *c,
                                     const String &data)
{
  LOG_INFO("Handle Command Update");

  if (m_hwController.state() == HardwareController::State::Idle) {
    m_config.firmware.setStartUpdate(true);
  }
}

/**
 * @brief Handles events from the MQTT connection, such as opening, messages,
 * and errors.
 * @param c The MQTT connection.
 * @param ev The event type.
 * @param ev_data Additional event data.
 */
void MqttClient::eventHandler(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_OPEN) {
    LOG_INFO("MQTT Opened");
  } else if (ev == MG_EV_ERROR) {
    MG_ERROR(("%lu ERROR %s", c->id, (char *) ev_data));
    c->is_draining = 1; // Mark connection for closure
  } else if (ev == MG_EV_MQTT_OPEN) {
    LOG_INFO("MQTT Connected, resetting fail count");
    m_failCount = 0;
    m_connected = true;
    m_lastPublishTimestamp_ms = 0;
  } else if (ev == MG_EV_MQTT_MSG) {
    struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
    String topic = String(mm->topic.buf, mm->topic.len);
    String data = String(mm->data.buf, mm->data.len);
    onMessageReceived(c, topic, data);
    c->recv.len = 0; // Clear received data
  } else if (ev == MG_EV_POLL) {
    if (millis() - m_lastMqttPing_ms >= MQTT_PING_INTERVAL_MS) {
      mg_mqtt_ping(c);
      m_lastMqttPing_ms = millis();
    }
  } else if (ev == MG_EV_CLOSE) {
    close();
  }
}