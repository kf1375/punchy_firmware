#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <mongoose.h>
#include <mongoose_config.h>

#include "configuration.h"
#include "dns_server.h"
#include "mqtt_client.h"
#include "web_server.h"
#include "web_updater.h"

#include "hardware_controller.h"
//************************************************************************************************
// Network Manager Class
// This class combines all network-related operations in the system. It handles
// WiFi connectivity, DNS services, MQTT communication, time synchronization,
// web server operations, and firmware updates. The `NetworkManager`
// orchestrates these components, ensuring a seamless connection to the cloud
// and efficient local network management.
//************************************************************************************************

class NetworkManager
{
public:
  NetworkManager(Configuration &config, HardwareController &hwController);
  ~NetworkManager();

  void setup();
  void loop();

private:
  enum WiFiState {
    INIT_STATE,
    AP_MODE,
    CONNECTION_ONGOING,
    CONNECT_TO_CLOUD,
    CONNECTED_TO_CLOUD,
    WAIT_BEFORE_NEW_CONNECTION,
  };

  Configuration &m_config;
  HardwareController &m_hwController;

  DnsServer m_dnsServer;
  WebServer m_webServer;
  MqttClient m_mqttClient;
  WebUpdater m_webUpdater;

  void connectToWifi();

  void changeState(WiFiState newState);
  void handleState();
  void printCurrentState();
  void printDecisionFlags();
  void WiFiEvent(WiFiEvent_t event);

  void startAP();
  void stopAP();
  void clearWifiConnection();
  void setupMqtt();
  void clearMqttConnection();

  void changeStateInitState();
  void changeStateApMode();
  void changeStateConnectionOngoing();
  void changeStateConnectToCloud();
  void changeStateConnectedToCloud();
  void changeStateWaitBeforeNewConnection();

  struct mg_mgr m_mgr;
  WiFiState m_currentState;
  unsigned long m_stateStartedAt = 0;
};

#endif // NETWORK_MANAGER_H
