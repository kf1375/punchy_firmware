#include "network_manager.h"

#include <WiFi.h>

#include "fs_interface.h"
#include "hardware_controller.h"
#include "util.h"

NetworkManager::NetworkManager() 
    : m_httpConnection(nullptr), m_mqttConnection(nullptr), 
      m_wifiConnected(false), m_hotspotEnabled(false), m_mqttConnected(false) 
{
    m_deviceId = Util::getMacAddress();
    Serial.println("Device ID: " + m_deviceId);
}

NetworkManager::~NetworkManager()
{
    stopHttpServer();
    // disconnectMqtt();
    disableHotspot();
    mg_mgr_free(&m_mgr);
}

void NetworkManager::init()
{
    Serial.println("Initialize Network Manager");

    WiFi.mode(WIFI_MODE_STA);
    
    m_fs = {
        .st = FsInterface::littlefs_stat,
        .ls = FsInterface::littlefs_ls,
        .op = FsInterface::littlefs_op,
        .cl = FsInterface::littlefs_cl,
        .rd = FsInterface::littlefs_rd,
        .wr = FsInterface::littlefs_wr,
        .sk = FsInterface::littlefs_sk,
        .mv = FsInterface::littlefs_mv,
        .rm = FsInterface::littlefs_rm,
        .mkd = FsInterface::littlefs_mkd,
    };
    
    m_httpServeOpts = {
        .root_dir = "/",
        .ssi_pattern = "",
        .extra_headers = "",
        .mime_types = "",
        .page404 = NULL,
        .fs = &m_fs,
    };

    mg_log_set(MG_LL_INFO);
    mg_log_set_fn([](char ch, void *) { Serial.print(ch); }, NULL);

    mg_mgr_init(&m_mgr);

    Serial.println("Initialize Network Done!");
}

void NetworkManager::connectWifi(const String &ssid, const String &password) 
{
    //     String ssid, password;
    // if (readCredentials(ssid, password)) {
    // search for ssid
    Serial.println("Scanning for WiFi networks...");
    WiFi.disconnect(true);
    int n = WiFi.scanNetworks();
    Serial.println("Scan done.");
    bool ssidFound = false;
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        for (int i = 0; i < n; ++i) {
            if (WiFi.SSID(i) == ssid) {
                ssidFound = true;
                break;
            }
        }
    }

    // Attempt to connect to the stored SSID
    if (ssidFound) {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("Connecting to WiFi");
        int counter = 0;
        while (WiFi.status() != WL_CONNECTED && counter < 20) {
            delay(500);
            Serial.print(".");
            counter++;
        }
        Serial.print("\n");
        if (WiFi.status() == WL_CONNECTED) {
            m_wifiConnected = true;
            Serial.println("Connected to: " + ssid);
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());

            // starting http server
            // String httpUrl = "http://" + WiFi.localIP().toString() + ":" + String(HTTP_PORT);
            // Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
            // mg_http_listen(&m_mgr, httpUrl.c_str(), httpEventHandler, NULL);

            // starting https server
            // String httpsUrl = "https://" + WiFi.localIP().toString() + ":" + String(HTTPS_PORT);
            // Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " +
            //                "Starting HTTPS listener on " + httpsUrl);
            // mg_http_listen(&mgr, httpsUrl.c_str(), httpEventHandler, NULL);

            // starting http websocket
            // String socketUrl = "http://" + WiFi.localIP().toString() + ":" + String(WS_PORT);
            // mg_http_listen(&mgr, socketUrl.c_str(), WebSocket::event_handler, NULL);

            // starting https websocket
            // String secureSockteUrl = "https://" + WiFi.localIP().toString() + ":" + String(WSS_PORT);
            // mg_http_listen(&mgr, secureSockteUrl.c_str(), WebSocket::event_handler, NULL);

            // Serial.println(String(__FILE__) + ":" + String(__LINE__) +
            //                ": Starting WS listener on " + socketUrl);
        } else {
            Serial.println("Failed to connect to WiFi");
        }
    } else {
        Serial.println("Target SSID not found");
    }
    // } else {
    //     Serial.println("No stored WiFi credentials found");
    // }
}

bool NetworkManager::isWifiConnected() const 
{
    return m_wifiConnected;
}

void NetworkManager::enableHotspot(const String &ssid, const String &password)
{
    WiFi.softAP(ssid.c_str(), password.isEmpty() ? nullptr : password.c_str());
    m_hotspotEnabled = true;
    Serial.println("Hotspot enabled. SSID: " + ssid + " IP: " + WiFi.softAPIP());
}

void NetworkManager::disableHotspot() 
{
    if (m_hotspotEnabled) {
        WiFi.softAPdisconnect(true);
        m_hotspotEnabled = false;
        Serial.println("Hotspot disabled.");
    }
}

void NetworkManager::startDnsServer()
{
    if (!m_hotspotEnabled) {
        Serial.println("Hotspot is not enabled. Cannot start DNS server.");
        return;
    }

    m_dnsUrl = "udp://" + WiFi.softAPIP().toString() + ":" + String(53);
    Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting DNS listener on " + m_dnsUrl);
    m_dnsConnection = mg_listen(&m_mgr, m_dnsUrl.c_str(), dnsEventHandler, this);

    if (m_httpConnection == nullptr) {
        Serial.println("Failed to start DNS server on " + m_httpUrl);;
    }
}

void NetworkManager::stopDnsServer()
{
    if (m_dnsConnection) {
        m_dnsConnection = nullptr;
    }
}

void NetworkManager::startHttpServer() 
{
    if (!m_hotspotEnabled) {
        Serial.println("Hotspot is not enabled. Cannot start HTTP server.");
        return;
    }

    m_httpUrl = "http://" + WiFi.softAPIP().toString() + ":" + String(80);
    Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + m_httpUrl);
    m_httpConnection = mg_http_listen(&m_mgr, m_httpUrl.c_str(), httpEventHandler, this);

    if (m_httpConnection == nullptr) {
        Serial.println("Failed to start HTTP server on " + m_httpUrl);;
    }
}

void NetworkManager::stopHttpServer() 
{
    if (m_httpConnection) {
        m_httpConnection = nullptr;
    }
}

void NetworkManager::connectMqttBroker()
{
    if (m_mqttConnection == nullptr) {
        if (!m_wifiConnected) {
            m_mqttReconnectAttempts++;
            Serial.println("Wifi is not enabled. Cann not connect to the MQTT broker.");
            return;
        }

        // memset(&m_mqttOpts, 0, sizeof(m_mqttOpts));
        // m_mqttOpts.keepalive = 60;
        // m_mqttOpts.client_id = mg_str("Jinx");
        // m_mqttOpts.clean = true;

        struct mg_mqtt_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.clean = true;
        opts.keepalive = 60;
        opts.client_id = mg_str(m_deviceId.c_str());
        m_brokerUrl = "mqtt://myremotedevice.com/mqtt:443";
        // m_brokerUrl = "mqtt://82.165.150.67:1883";
        Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Connecting to: " + m_brokerUrl);
        m_mqttConnection = mg_mqtt_connect(&m_mgr, m_brokerUrl.c_str(), &opts, mqttEventHandler, this);

        if (m_mqttConnection == nullptr) {
            Serial.println("Failed to connect MQTT broker: " + m_brokerUrl);
        }
    }
}

void NetworkManager::disconnectMqtt()
{
    if (m_mqttConnection) {
        m_mqttConnection = nullptr;
        m_mqttReconnectAttempts++;
    }

    Command cmd;
    cmd.type = CommandType::STOP;
    cmd.value = 0;

    if (!m_commandQueue->addCommand(cmd)) {
        Serial.println("Queue is full");
    }
}

void NetworkManager::mqttTlsInit(struct mg_connection *c)
{
    if (mg_url_is_ssl(m_brokerUrl.c_str())) {
        // struct mg_tls_opts opts = {.ca = "ca.pem"};
        // mg_tls_init(c, &opts);
        Serial.println("Target URL is SSL/TLS, command client connection to use TLS.");
    }
}

void NetworkManager::onMqttConnected(struct mg_connection *c, int code)
{
    struct mg_mqtt_opts opts;
    memset(&opts, 0, sizeof(opts));
    opts.qos = 0;
    opts.clean = true;
    opts.topic = mg_str((m_deviceId + "/pair").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/status").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/unpair").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/start/single").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/start/infinite").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/stop").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/settings/turn_type").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/settings/set_front").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/settings/set_rear").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/settings/max_half_speed").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/settings/max_full_speed").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/commands/up").c_str());
    mg_mqtt_sub(c, &opts);
    opts.topic = mg_str((m_deviceId + "/commands/down").c_str());
    mg_mqtt_sub(c, &opts);
    Serial.println("Subscribing to " + m_deviceId + "/#");
}

void NetworkManager::onMqttMessageReceived(struct mg_connection *c, struct mg_str *topic, struct mg_str *data)
{
    struct mg_str caps[3];
    
    if (mg_match(*topic, mg_str((m_deviceId + "/#/#").c_str()), caps)) {
        struct mg_str *second_level = &caps[1];
        if (mg_strcasecmp(caps[0], mg_str("start")) == 0) {
            mqttHandleStart(c, second_level, data);
        }  else if (mg_strcasecmp(caps[0], mg_str("settings")) == 0) {
            mqttHandleSettings(c, second_level, data);
        } else if (mg_strcasecmp(caps[0], mg_str("commands")) == 0) {
            mqttHandleCommands(c, second_level, data);
        } else {
            mqttHandleUnknown(c, data);
        }
    } else if (mg_match(*topic, mg_str((m_deviceId + "/#").c_str()), caps)) {
        if (mg_strcasecmp(caps[0], mg_str("pair")) == 0) {
            mqttHandlePair(c, data);
        } else if (mg_strcasecmp(caps[0], mg_str("status")) == 0) {
            mqttHandleStatus(c, data);
        } else if (mg_strcasecmp(caps[0], mg_str("unpair")) == 0) {
            mqttHandleUnpair(c, data);
        } else if (mg_strcasecmp(caps[0], mg_str("stop")) == 0) {
            mqttHandleStop(c, data);
        } else {
            mqttHandleUnknown(c, data);
        }
    } else {
        mqttHandleUnknown(c, data);
    }
}

void NetworkManager::mqttHandlePair(struct mg_connection *c, struct mg_str *data)
{
    Serial.println("Handle Pair");

    if (data->buf == nullptr) {
        Serial.println("Received null data");
        return;
    }

    struct mg_str json = *data;
    
    char *type = NULL;
    type = mg_json_get_str(json, "$.type");
    if (!type) {
        Serial.println("Error: 'type' field not found");
        return;
    }

    if (strcmp(type, "request") == 0) {
        char *name = mg_json_get_str(json, "$.name");
        if (!name) {
            Serial.println("Error: 'name' field not found");
            free(type);
            return;
        }

        Serial.printf("Pairing request received for device: %s\n", name);

        struct mg_mqtt_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.qos = 0;
        opts.topic = mg_str((m_deviceId + "/pair").c_str());
        opts.message = mg_str("{\"type\":\"response\",\"status\":\"accepted\",\"message\":\"Device paired successfully\"}");
        if (m_mqttConnection) {
            mg_mqtt_pub(m_mqttConnection, &opts);
        }
        free(name);
        Serial.println("Pairing response published successfully");
    }

    free(type);
}

void NetworkManager::mqttHandleStatus(struct mg_connection *c, struct mg_str *data)
{
    Serial.println("Handle Status");
}

void NetworkManager::mqttHandleUnpair(struct mg_connection *c, struct mg_str *data)
{
    Serial.println("Handle Unpair");
}

void NetworkManager::mqttHandleStart(struct mg_connection *c, struct mg_str *mode, struct mg_str *data)
{
    Serial.println("Handle Start");

    if (data->buf == nullptr) {
        Serial.println("Received null data");
        return;
    }

    struct mg_str json = *data;

    int32_t speed = mg_json_get_long(json, "$.speed", -1);
    if (speed == -1) {
        Serial.println("Error: 'type' field to get speed");
        return;
    }

    Command cmd;
    cmd.value = speed;
    if (mg_strcmp(*mode, mg_str("single")) == 0) {
        cmd.type = CommandType::START_SIGNLE;
    } else if (mg_strcmp(*mode, mg_str("infinite")) == 0) {
        cmd.type = CommandType::START_INFINITE;
    } else {
        Serial.println("Invalid Start Command");
        return;
    }
    
    if (!m_commandQueue->addCommand(cmd)) {
        Serial.println("Queue is full");
    }
}

void NetworkManager::mqttHandleStop(struct mg_connection *c, struct mg_str *data)
{
    (void) data;
    Serial.println("Handle Stop");

    Command cmd;
    cmd.type = CommandType::STOP;
    cmd.value = 0;

    if (!m_commandQueue->addCommand(cmd)) {
        Serial.println("Queue is full");
    }
}

void NetworkManager::mqttHandleSettings(struct mg_connection *c, struct mg_str *param, struct mg_str *data)
{
    Serial.println("Handle Settings");

    if (param->buf == nullptr || data->buf == nullptr) {
        Serial.println("Received null data");
        return;
    }

    struct mg_str json = *data;

    Command cmd;
    if (mg_strcmp(*param, mg_str("turn_type")) == 0) {
        cmd.type = CommandType::SETTING_TURN_TYPE;
        char *turn_type = NULL;
        turn_type = mg_json_get_str(json, "$.value");
        if (!turn_type) {
            Serial.println("Error: 'value' field not found");
            return;
        }
        if (strcmp(turn_type, "Full Turn") == 0) {
            cmd.value = (int32_t) TurnType::FULL_TURN;
        } else if (strcmp(turn_type, "Half Turn") == 0) {
            cmd.value = (int32_t) TurnType::HALF_TURN;
        }
        free(turn_type);
    } else if (mg_strcmp(*param, mg_str("set_front")) == 0) {
        cmd.type = CommandType::SETTING_SET_FRONT;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else if (mg_strcmp(*param, mg_str("set_rear")) == 0) {
        cmd.type = CommandType::SETTING_SET_REAR;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else if (mg_strcmp(*param, mg_str("max_half_speed")) == 0) {
        cmd.type = CommandType::SETTING_MAX_HALF_SPEED;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else if (mg_strcmp(*param, mg_str("max_full_speed")) == 0) {
        cmd.type = CommandType::SETTING_MAX_FULL_SPEED;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else {
        Serial.println("Invalid Setting");
        return;
    }

    if (!m_commandQueue->addCommand(cmd)) {
        Serial.println("Queue is full");
    }
}

void NetworkManager::mqttHandleCommands(struct mg_connection *c, struct mg_str *command, struct mg_str *data)
{
    Serial.println("Handle Commands");

    if (command->buf == nullptr || data->buf == nullptr) {
        Serial.println("Received null data");
        return;
    }

    struct mg_str json = *data;

    Command cmd;
    if (mg_strcmp(*command, mg_str("up")) == 0) {
        cmd.type = CommandType::COMMAND_UP;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else if (mg_strcmp(*command, mg_str("down")) == 0) {
        cmd.type = CommandType::COMMAND_DOWN;
        cmd.value = mg_json_get_long(json, "$.value", -1);
    } else {
        Serial.println("Invalid Command");
        return;
    }

    if (!m_commandQueue->addCommand(cmd)) {
        Serial.println("Queue is full");
    }
}

void NetworkManager::mqttHandleUnknown(struct mg_connection *c, struct mg_str *data)
{
    Serial.println("Unknown mqtt message received.");
}

void NetworkManager::poll() 
{
    mg_mgr_poll(&m_mgr, 1000);
    update();
}

void NetworkManager::dnsEventHandler(struct mg_connection *c, int ev, void *ev_data) 
{
    (void)ev_data;
    auto networkManager = static_cast<NetworkManager *>(c->fn_data);
    if (ev == MG_EV_OPEN) {
        c->is_hexdumping = 1;
    } else if (ev == MG_EV_READ) {
        struct mg_dns_rr rr; // Parse first question, offset 12 is header size
        size_t n = mg_dns_parse_rr(c->recv.buf, c->recv.len, 12, true, &rr);
        MG_INFO(("DNS request parsed, result=%d", (int)n));
        if (n > 0) {
            char buf[512];
            struct mg_dns_header *h = (struct mg_dns_header *)buf;
            memset(buf, 0, sizeof(buf));                                  // Clear the whole datagram
            h->txnid = ((struct mg_dns_header *)c->recv.buf)->txnid;      // Copy tnxid
            h->num_questions = mg_htons(1);                               // We use only the 1st question
            h->num_answers = mg_htons(1);                                 // And only one answer
            h->flags = mg_htons(0x8400);                                  // Authoritative response
            memcpy(buf + sizeof(*h), c->recv.buf + sizeof(*h), n);        // Copy question
            memcpy(buf + sizeof(*h) + n, dns_answer, sizeof(dns_answer)); // And answer
            mg_send(c, buf, 12 + n + sizeof(dns_answer));                 // And send it!
        }
        mg_iobuf_del(&c->recv, 0, c->recv.len);
    } else if (ev == MG_EV_CLOSE) {
        
        networkManager->stopDnsServer();
    }
}

void NetworkManager::httpEventHandler(struct mg_connection *c, int ev, void *ev_data)  
{
    auto networkManager = static_cast<NetworkManager *>(c->fn_data);
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        mg_http_serve_opts *httpServeOpts = networkManager->httpServeOpts();
        if (mg_match(hm->uri, mg_str("/hotspot-detect.html"), NULL) ||
            mg_match(hm->uri, mg_str("/generate_204"), NULL) ||
            mg_match(hm->uri, mg_str("/gen_204"), NULL)) 
        {
            mg_http_serve_file(c, hm, "/captive_page.html", httpServeOpts);
        } else if (mg_match(hm->uri, mg_str("/send-config"), NULL)) {
            networkManager->httpHandleSetWifiConfig(c, hm);
        } else if (mg_match(hm->uri, mg_str("/wifi-state"), NULL)) {
            networkManager->httpHandleGetWifiStatus(c, hm);
        } else {
            mg_http_serve_dir(c, hm, httpServeOpts);
        }
        // Delete received data
        c->recv.len = 0;
        c->is_draining = 1;
    } else if (ev == MG_EV_ACCEPT) {
        // on https start tls handshaking
        // if (c->loc.port == mg_htons(HTTPS_PORT)) {
        //   mg_tls_init(c, &s_tls_opts);
        // }
        // if (sizeof(c->mgr->conns) > LIMIT) {
        if (numberOfConnections(c->mgr) > 20) {
            Serial.println("Too many connections");
            c->is_closing = 1;
        }
    } else if (ev == MG_EV_ERROR) {
        // on error flush the mg_io_buff
        c->is_closing = 1;
    } else if (ev == MG_EV_CLOSE) {
        networkManager->stopHttpServer();
    }
}

void NetworkManager::mqttEventHandler(struct mg_connection *c, int ev, void *ev_data)
{
    auto networkManager = static_cast<NetworkManager *>(c->fn_data);
    if (ev == MG_EV_OPEN) {
        Serial.println("New connection created: " + c->id);
    } else if (ev == MG_EV_ERROR) {
        Serial.println("Error: " + c->id + String((char *) ev_data));
    } else if (ev == MG_EV_CONNECT) {
        // If target URL is SSL/TLS, command client connection to use TLS
        networkManager->mqttTlsInit(c);
    } else if (ev == MG_EV_MQTT_OPEN) {
        Serial.println("MQTT connect is successful.");
        networkManager->onMqttConnected(c, *(int *) ev_data);
    } else if (ev == MG_EV_MQTT_MSG) {
        struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
        networkManager->onMqttMessageReceived(c, &mm->topic, &mm->data);
    } else if (ev == MG_EV_CLOSE) {
        Serial.println("MQTT closed.");
        networkManager->disconnectMqtt();
    } else if (ev == MG_EV_POLL) {
        if(millis() - networkManager->lastMqttPingMillis() >= MQTT_PING_INTERVAL_MS) { 
            mg_mqtt_ping(c); 
            networkManager->setLastMqttPingMillis(millis()); 
        }
    }
}

void NetworkManager::update()
{
    unsigned long currentTimestamp_ms = millis();
    if ((currentTimestamp_ms - m_lastTimestamp_ms) > NETWORK_UPDATE_INTERVAL_MS) {
        m_lastTimestamp_ms = currentTimestamp_ms;
        if (m_mqttConnection == nullptr) {
            connectMqttBroker();
        }
    }
}

void NetworkManager::httpHandleSetWifiConfig(struct mg_connection *c, struct mg_http_message *hm) {
    // char param[128];
    // char nssid[128];
    // char npassword[128];
    // if (mg_http_get_var(&hm->body, "ssid", param, sizeof(param)) <= 0) {
    //     Serial.println("Missing 'ssid' parameter");
    //     mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'ssid' parameter");
    //     return;
    // }
    // strcpy(nssid, param);
    // if (mg_http_get_var(&hm->body, "password", param, sizeof(param)) <= 0) {
    //     Serial.println("Missing 'password' parameter");
    //     mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'password' parameter");
    //     return;
    // }
    // strcpy(npassword, param);

    // Serial.println(nssid);
    // Serial.println(npassword);

    // // Connect to the new Wi-Fi network
    // // search for ssid
    // Serial.println("Scanning for WiFi networks...");
    // int n = WiFi.scanNetworks();
    // Serial.println("Scan done.");
    // bool ssidFound = false;
    // if (n == 0) {
    //     Serial.println("No networks found");
    // } else {
    //     for (int i = 0; i < n; ++i) {
    //         Serial.println("New SSID found: " + String(WiFi.SSID(i)));
    //         if (WiFi.SSID(i) == String(nssid)) {
    //             ssidFound = true;
    //             break;
    //         }
    //     }
    // }

    // // Attempt to connect to the stored SSID
    // if (ssidFound) {
    //     WiFi.begin(nssid, npassword);

    //     // Wait for connection
    //     unsigned long startTime = millis();
    //     const unsigned long timeout = 10000; // 10 seconds timeout
    //     while (WiFi.status() != WL_CONNECTED) {
    //         if (millis() - startTime >= timeout) {
    //             Serial.println("Failed to connect to WiFi: Timeout");
    //             mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Failed to connect to WiFi: Timeout");
    //             return;
    //         }
    //         delay(1000);
    //         Serial.println("Connecting to WiFi...");
    //     }
    //     Serial.println("Connected to WiFi");

    //     // Save the new WiFi credentials to the wifi.txt file
    //     if (saveCredentials(String(nssid), String(npassword))) {
    //         Serial.println("WiFi credentials saved");
    //     } else {
    //         Serial.println("Failed to save WiFi credentials");
    //     }

    //     //************* Re-Init Web Server *******************
    //     // starting http server
    //     String httpUrl = "http://" + WiFi.localIP().toString() + ":" + String(HTTP_PORT);
    //     Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
    //     mg_http_listen(&mgr, httpUrl.c_str(), httpEventHandler, NULL);

        // starting https server
        // String httpsUrl = "https://" + WiFi.localIP().toString() + ":" + String(HTTPS_PORT);
        // Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " +
        //                "Starting HTTPS listener on " + httpsUrl);
        // mg_http_listen(&mgr, httpsUrl.c_str(), httpEventHandler, NULL);

        // starting http websocket
        // String socketUrl = "http://" + WiFi.localIP().toString() + ":" + String(WS_PORT);
        // mg_http_listen(&mgr, socketUrl.c_str(), WebSocket::event_handler, NULL);

        // starting https websocket
        // String secureSockteUrl = "https://" + WiFi.localIP().toString() + ":" + String(WSS_PORT);
        // mg_http_listen(&mgr, secureSockteUrl.c_str(), WebSocket::event_handler, NULL);

        // Serial.println(String(__FILE__) + ":" + String(__LINE__) +
        //                ": Starting WS listener on " + socketUrl);

        // Respond to the client with the IP address
    //     String ipAddress = WiFi.localIP().toString();
    //     mg_http_reply(c, 200, "Content-Type: text/plain\r\n", ipAddress.c_str());
    // } else {
    //     Serial.println("Target SSID not found");
    //     mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Failed to connect to WiFi: Target SSID not found");
    // }
}

void NetworkManager::httpHandleGetWifiStatus(struct mg_connection *c, struct mg_http_message *hm) {
    // String response = "{\"status\":\"";
    // if (WiFi.status() == WL_CONNECTED) {
    //     response += "connected\",\"ssid\":\"" + WiFi.SSID() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    // } else {
    //     response += "disconnected\"}";
    // }
    // mg_http_reply(c, 200, "Content-Type: application/json\r\n", response.c_str());
}

int NetworkManager::numberOfConnections(struct mg_mgr *mgr) {
    int n = 0;
    for (struct mg_connection *t = mgr->conns; t != NULL; t = t->next)
        n++;
    return n;
}