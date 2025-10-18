#include "network_manager.h"

#include "util.h"
#include <WiFi.h>

void NetworkManager::WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
  case SYSTEM_EVENT_STA_START:
    LOG_INFO("Wi-Fi started.");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    m_config.wifi.setFailed(true);
    LOG_INFO("Disconnected from Wi-Fi. Possible wrong password or SSID.");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    LOG_INFO("Connected to Wi-Fi network.");
    m_config.wifi.setFailed(false);
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    LOG_INFO("Connected to " + m_config.wifi.ssid() +
             " and got IP: " + WiFi.localIP().toString());
    break;
  default:
    LOG_INFO("Wi-Fi event: " + String(event));
    break;
  }
}

NetworkManager::NetworkManager(Configuration &config,
                               HardwareController &hwController)
    : m_config(config),
      m_hwController(hwController),
      m_dnsServer(m_mgr),
      m_webServer(m_mgr, m_config, m_hwController),
      m_mqttClient(m_mgr, m_config, m_hwController),
      m_webUpdater(m_config)
{
}

NetworkManager::~NetworkManager() {}

/**
 * @brief Setup network manager
 *
 * Start mongoose, start WiFi setup by triggering an initial scan and setup AP
 * if no credentials are stored
 */
void NetworkManager::setup()
{
  mg_log_set(MG_LL_INFO);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
    this->WiFiEvent(event);
  });

  mg_mgr_init(&m_mgr);
}

/**
 * @brief Network Loop, called by the second core continuously
 *
 * Work on mongoose events as well as event loops of subtasks
 * Whenever wifi connection is lost, attempt a reconnection every 5 Minutes, if
 * credentials are stored. Make sure that AP is running if no credentials are
 * stored
 */
void NetworkManager::loop()
{
  mg_mgr_poll(&m_mgr, 1000);

  static unsigned long last_alive_msg = 0;
  if (Util::timeElapsed(60, last_alive_msg)) {
    last_alive_msg = millis();
    printCurrentState();
  }

  handleState();
}

/**
 * @brief Start Wifi Access Point with specific settings
 *
 * Do NOT change the address ranges! This will break functionality with
 * at least Samsung Devices!
 *
 */
void NetworkManager::startAP()
{
  static bool ap_started = false;
  if (ap_started)
    return;

  // run this function only once.
  ap_started = true;
  // For Access Point Mode:
  // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
  String ap_ssid = "Testiwhisk_" + Util::getMacAddress();
  // Use this IP range to for more reliability on samsung devices
  IPAddress ap_ip(172, 217, 28, 1);
  IPAddress ap_mask(255, 255, 255, 0);
  IPAddress ap_leaseStart(172, 217, 28, 2);

  WiFi.softAP(ap_ssid.c_str(), "12345678");
  WiFi.setMinSecurity(WIFI_AUTH_WPA2_PSK);
  delay(100); // you have to wait until event SYSTEM_EVENT_AP_START fireed
  // define AP-DHCP Settings for more reliable AP connection on samsung devices
  WiFi.softAPConfig(ap_ip, ap_ip, ap_mask, ap_leaseStart);
  LOG_INFO("Access Point Started. IP: " + WiFi.softAPIP().toString());

  m_webServer.setup();
  m_dnsServer.setup();
}

void NetworkManager::stopAP()
{
  m_dnsServer.~DnsServer();
}

/**
 * @brief Connect to the stored wifi network
 */
void NetworkManager::connectToWifi()
{
  // Attempt to connect to the stored SSID
  m_config.wifi.setFailed(false);
  String ssid = m_config.wifi.ssid();
  String pass = m_config.wifi.password();
  LOG_INFO("Connecting to WiFi " + ssid);
  WiFi.begin(ssid.c_str(), pass.c_str());
  delay(100); // you have to wait until event SYSTEM_EVENT_STA_START fired
}

void NetworkManager::changeState(WiFiState newState)
{
  m_stateStartedAt = millis();
  m_currentState = newState;
  printCurrentState();
}

void NetworkManager::handleState()
{
  switch (m_currentState) {
  case INIT_STATE:
    changeStateInitState();
    break;
  case AP_MODE:
    changeStateApMode();
    break;
  case CONNECTION_ONGOING:
    changeStateConnectionOngoing();
    break;
  case CONNECT_TO_CLOUD:
    changeStateConnectToCloud();
    break;
  case CONNECTED_TO_CLOUD:
    changeStateConnectedToCloud();
    break;
  case WAIT_BEFORE_NEW_CONNECTION:
    changeStateWaitBeforeNewConnection();
    break;
  default:
    break;
  }
}

void NetworkManager::printCurrentState()
{
  LOG_INFO("Current State: ");
  switch (m_currentState) {
  case INIT_STATE:
    LOG_INFO("INIT_STATE");
    break;
  case AP_MODE:
    LOG_INFO("AP_MODE");
    break;
  case CONNECTION_ONGOING:
    LOG_INFO("CONNECTION_ONGOING");
    break;
  case CONNECT_TO_CLOUD:
    LOG_INFO("CONNECT_TO_CLOUD");
    break;
  case CONNECTED_TO_CLOUD:
    LOG_INFO("CONNECTED_TO_CLOUD");
    break;
  case WAIT_BEFORE_NEW_CONNECTION:
    LOG_INFO("WAIT_BEFORE_NEW_CONNECTION");
    break;
  default:
    LOG_INFO("UNKNOWN");
    break;
  }
  printDecisionFlags();
}

void NetworkManager::printDecisionFlags()
{
  LOG_INFO("-------------------------------------------------------------------"
           "----");
  LOG_INFO("WiFi Connected: " + String(WiFi.status() == WL_CONNECTED));
  if (m_config.wifi.stored())
    LOG_INFO("WiFi Stored: " + String(m_config.wifi.stored()));
  if (!m_mqttClient.isClientConnected())
    LOG_INFO("MQTT Connected: " + String(m_mqttClient.isClientConnected()));
  if (m_config.wifi.failed())
    LOG_INFO("Connection Failed: " + String(m_config.wifi.failed()));
  LOG_INFO("-------------------------------------------------------------------"
           "----");
}

void NetworkManager::clearWifiConnection()
{
  WiFi.disconnect(true);
}

void NetworkManager::setupMqtt()
{
  m_mqttClient.setup();
  m_webServer.setup();
  m_webUpdater.setup();
}

void NetworkManager::clearMqttConnection()
{
  LOG_INFO("Clearing MQTT Connection");
  m_mqttClient.stop();
  m_mqttClient.~MqttClient();
}

void NetworkManager::changeStateInitState()
{
  LOG_INFO("Starting network manager...");
  m_hwController.setLEDState(HardwareController::LEDState::Orange);
  if (m_config.wifi.stored()) {
    connectToWifi();
    changeState(CONNECTION_ONGOING);
  } else {
    startAP();
    changeState(AP_MODE);
  }
}

void NetworkManager::changeStateApMode()
{
  if (m_config.wifi.stored()) {
    connectToWifi();
    changeState(CONNECTION_ONGOING);
  } else if (WiFi.status() == WL_CONNECTED) {
    changeState(CONNECTION_ONGOING);
  }
  m_webServer.loop();
  m_dnsServer.loop();
}

void NetworkManager::changeStateConnectionOngoing()
{
  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    setupMqtt();
    changeState(CONNECT_TO_CLOUD);
  } else if (m_config.wifi.failed()) {
    WiFi.disconnect();
    LOG_INFO("Clearing WiFi Credentials and starting AP");
    LOG_INFO("failed: " + String(m_config.wifi.failed()) +
             " status: " + String(status));
    startAP();
    m_config.wifi.reset();
    changeState(AP_MODE);
    m_hwController.setLEDState(HardwareController::LEDState::Orange);
  }
}

void NetworkManager::changeStateConnectToCloud()
{
  if (WiFi.status() != WL_CONNECTED) {
    clearMqttConnection();
    clearWifiConnection();
    changeState(WAIT_BEFORE_NEW_CONNECTION);
    m_hwController.setLEDState(HardwareController::LEDState::Orange);
  } else if (m_mqttClient.isClientConnected()) {
    changeState(CONNECTED_TO_CLOUD);
    m_hwController.setLEDState(HardwareController::LEDState::Green);
  } else if (m_mqttClient.failed()) {
    LOG_INFO("MQTT Connection Failed, Resetting System");
    assert(false);
  }
}

void NetworkManager::changeStateConnectedToCloud()
{
  if (WiFi.status() != WL_CONNECTED) {
    clearMqttConnection();
    clearWifiConnection();
    changeState(WAIT_BEFORE_NEW_CONNECTION);
    m_hwController.setLEDState(HardwareController::LEDState::Orange);
  } else if (!m_mqttClient.isClientConnected()) {
    clearMqttConnection();
    setupMqtt();
    changeState(CONNECT_TO_CLOUD);
    m_hwController.setLEDState(HardwareController::LEDState::Orange);
  } else { 
    m_mqttClient.run();
    if (m_hwController.state() == HardwareController::State::Idle ||
        m_hwController.state() == HardwareController::State::Stop) {
      m_webUpdater.loop();
    }
  }
}

void NetworkManager::changeStateWaitBeforeNewConnection()
{
  if (Util::timeElapsed(60, m_stateStartedAt)) {
    // WiFi.mode(WIFI_AP_STA);
    connectToWifi();
    changeState(CONNECTION_ONGOING);
    m_hwController.setLEDState(HardwareController::LEDState::Orange);
  }
}
