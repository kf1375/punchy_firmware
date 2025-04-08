#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <mongoose.h>

#include "configuration.h"
#include "hardware_controller.h"

#define MQTT_PING_INTERVAL_MS 30 * 1000

class MqttClient
{
public:
  MqttClient(mg_mgr &mgr, Configuration &config,
             HardwareController &hwController);

  void setup();
  void run();
  void stop();

  bool isClientConnected();
  bool failed() { return m_failCount >= 5; };

private:
  Configuration &m_config;
  struct mg_mgr &m_mgr;
  HardwareController &m_hwController;

  struct mg_connection *m_mqttConn;
  String m_mqttPrefix;

  bool m_discoveryIsNecessary = true;
  size_t m_failCount = 0;
  bool m_stopped = false;
  bool m_connected = false;

  unsigned long m_lastMqttPing_ms = 0;

  void subscribe();
  void restart();
  void close();
  void publish(String topic, String data, bool retain = false);

  void onMessageReceived(struct mg_connection *c, const String &topic,
                         const String &data);
  void handlePair(struct mg_connection *c, const String &data);
  void handleStatus(struct mg_connection *c, const String &data);
  void handleUnpair(struct mg_connection *c, const String &data);
  void handleStartSingle(struct mg_connection *c, const String &data);
  void handleStartInfinite(struct mg_connection *c, const String &data);
  void handleStop(struct mg_connection *c, const String &data);
  void handleSetTurnType(struct mg_connection *c, const String &data);
  void handleSetHitPos(struct mg_connection *c, const String &data);
  void handleSetRestPos(struct mg_connection *c, const String &data);
  void handleSetMaxHalfSpeed(struct mg_connection *c, const String &data);
  void handleSetMaxFullSpeed(struct mg_connection *c, const String &data);
  void handleSetHitDirection(struct mg_connection *c, const String &data);
  void handleCmdLeft(struct mg_connection *c, const String &data);
  void handleCmdRight(struct mg_connection *c, const String &data);
  void handleCmdUpdate(struct mg_connection *c, const String &data);

  void eventHandler(struct mg_connection *c, int ev, void *ev_data);
};

#endif // MQTT_CLIENT_H