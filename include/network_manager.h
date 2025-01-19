#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <mongoose.h>
#include <mongoose_config.h>

#include "command_queue.h"

#define NETWORK_UPDATE_INTERVAL_MS 5000
#define MQTT_MAX_RETRIES 3

static uint8_t dns_answer[] = {
        0xc0, 0x0c,          // Point to the name in the DNS question
        0,    1,             // 2 bytes - record type, A
        0,    1,             // 2 bytes - address class, INET
        0,    0,    0, 120,  // 4 bytes - TTL
        0,    4,             // 2 bytes - address length
        192,  168,    4,   1   // 4 bytes - IP address
}; 

enum MqttTopics {
    MQTT_TOPIC_UNKNOWN = 0,
    MQTT_TOPIC_PAIR,
    MQTT_TOPIC_STATUS,
    MQTT_TOPIC_DISCONNECT,
    MQTT_TOPIC_START_SINGLE,
    MQTT_TOPIC_START_INFINITE,
    MQTT_TOPIC_STOP,
    MQTT_TOPIC_SETTINGS_TURN_TYPE,
    MQTT_TOPIC_SETTINGS_SET_FRONT,
    MQTT_TOPIC_SETTINGS_SET_REAR,
    MQTT_TOPIC_SETTINGS_MAX_HALF_SPEED,
    MQTT_TOPIC_SETTINGS_MAX_FULL_SPEED,
    MQTT_TOPIC_COMMANDS_UP,
    MQTT_TOPIC_COMMANDS_DOWN
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void init();
    // Wi-Fi Methods
    void connectWifi(const String &ssid, const String &password);
    bool isWifiConnected() const;
    void enableHotspot(const String &ssid, const String &password = "");
    void disableHotspot();
    
    // DNS Methods
    void startDnsServer();
    void stopDnsServer();

    // HTTP Methods
    void startHttpServer();
    void stopHttpServer();

    // MQTT Methods
    void connectMqttBroker();
    void disconnectMqtt();
    void mqttTlsInit(struct mg_connection *c);
    void onMqttConnected(struct mg_connection *c, int code);
    void onMqttMessageReceived(struct mg_connection *c, struct mg_str *topic, struct mg_str *data);

    void mqttHandlePair(struct mg_connection *c, struct mg_str *data);
    void mqttHandleStatus(struct mg_connection *c, struct mg_str *data);
    void mqttHandleDisconnect(struct mg_connection *c, struct mg_str *data);
    void mqttHandleStart(struct mg_connection *c, struct mg_str *mode, struct mg_str *data);
    void mqttHandleStop(struct mg_connection *c, struct mg_str *data);
    void mqttHandleSettings(struct mg_connection *c, struct mg_str *param, struct mg_str *data);
    void mqttHandleCommands(struct mg_connection *c, struct mg_str *command, struct mg_str *data);
    void mqttHandleUnknown(struct mg_connection *c, struct mg_str *data);

    mg_http_serve_opts* httpServeOpts() { return &m_httpServeOpts; };
    // mg_mqtt_opts* mqttOpts() { return &m_mqttOpts; };
    String brokerUrl() { return m_brokerUrl; };
    
    int mqttReconnectAttempts() { return m_mqttReconnectAttempts; };
    void setMqttReconnectAttempts(int cnt) { m_mqttReconnectAttempts = cnt; };

    void poll();

    void setCommandQueue(CommandQueue *queue) { m_commandQueue = queue; };

private:
    static void dnsEventHandler(struct mg_connection *c, int ev, void *ev_data);
    static void httpEventHandler(struct mg_connection *c, int ev, void *ev_data);
    void httpHandleSetWifiConfig(struct mg_connection *c, struct mg_http_message *hm); // Http endpoint to handle set WiFi configuration
    void httpHandleGetWifiStatus(struct mg_connection *c, struct mg_http_message *hm); // Http endpoint to handle get WiFi Status

    static void mqttEventHandler(struct mg_connection *c, int ev, void *ev_data);
    static inline int numberOfConnections(struct mg_mgr *mgr);

    void update();

    String m_deviceId;

    struct mg_mgr m_mgr;

    struct mg_http_serve_opts m_httpServeOpts;
    // struct mg_mqtt_opts m_mqttOpts;

    struct mg_fs m_fs;
    
    struct mg_connection *m_dnsConnection;
    struct mg_connection *m_httpConnection;
    struct mg_connection *m_mqttConnection;

    bool m_wifiConnected;
    bool m_hotspotEnabled;
    bool m_mqttConnected;

    String m_httpUrl;
    String m_dnsUrl;
    String m_brokerUrl;

    unsigned long m_lastTimestamp_ms = 0;

    uint8_t m_mqttReconnectAttempts = 0;

    CommandQueue *m_commandQueue;
};

#endif // NETWORK_MANAGER_H
