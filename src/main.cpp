#include "custom_functions.h"
#include "driver.h"
#include "fs_interface.h"
#include "global_variables.h"
#include "io.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <mongoose.h>
#include <mongoose_config.h>

// **********CREATE OBJECTS**********
// For stepper motor
driver stepper1;

// create a mongoose objects
struct mg_mgr mgr;
struct mg_http_serve_opts s_http_serve_opts;

// WiFi and MQTT client initialization
WiFiClientSecure client;
PubSubClient mqtt_client(client);

// **********ENUMS**********
enum toolType { FULLREV, HALFREV };
toolType tool_type = FULLREV;

enum turnMode { STOP, SINGLE, INFINITE };
turnMode turn_mode = STOP;

enum userInput { START_SINGLE, START_INFINITE, STOP_INFINITE, SETTINGS }; // in Json user_mode
userInput user_input = STOP_INFINITE;

enum motorState {
    SINGLE_START,  // starts the single turn
    SINGLE_ROTATE, // durning forward movement
    SINGLE_PAUSE,  // pause at hit position
    SINGLE_PAUSE_BACK,
    ROTATE_BACK, // moving from hit to home pos

    INFINITE_ROTATE, // starts the stepper1.rotate
    INFINITE_BACK    // brings the tool to the home pos after the rotate
};
motorState motor_state = SINGLE_START;

// **********FUNCTIONS**********
#pragma region Webserver functions
// define event handlers
static void dnsEventHandler(struct mg_connection *c, int ev, void *ev_data);
static void httpEventHandler(struct mg_connection *c, int ev, void *ev_data);
static void mqttEventHandler(char *topic, byte *payload, unsigned int length);

// define littlefs interface with function pointers for mongoose
static struct mg_fs littlefs_interface = {
    .st = littlefs_stat,
    .ls = littlefs_ls,
    .op = littlefs_op,
    .cl = littlefs_cl,
    .rd = littlefs_rd,
    .wr = littlefs_wr,
    .sk = littlefs_sk,
    .mv = littlefs_mv,
    .rm = littlefs_rm,
    .mkd = littlefs_mkd,
};

void connectToWiFi();
void connectToMQTT();

// functions for handling http requests
void handleAutomaticModeOn(struct mg_connection *c, mg_http_message *hm) {
    automatic_mode = true;
    Serial.println("automatic mode on");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleAutomaticModeOff(struct mg_connection *c, mg_http_message *hm) {
    automatic_mode = false;
    Serial.println("automatic mode off");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleHalf(struct mg_connection *c, mg_http_message *hm) {
    tool_type = HALFREV;
    Serial.println("HALFREV");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleFull(struct mg_connection *c, mg_http_message *hm) {
    tool_type = FULLREV;
    Serial.println("FULLREV");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSingleTurnStart(struct mg_connection *c, mg_http_message *hm) {
    user_input = START_SINGLE;
    Serial.println("Single turn started");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSingleSpeedPlus(struct mg_connection *c, mg_http_message *hm) {
    single_speed = single_speed + 20;
    Serial.println("Single speed increased");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSingleSpeedMinus(struct mg_connection *c, mg_http_message *hm) {
    single_speed = single_speed - 20;
    Serial.println("Single speed decreased");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleInfiniteTurnStart(struct mg_connection *c, mg_http_message *hm) {
    user_input = START_INFINITE;
    Serial.println("turn_mode = INFINITE");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleInfiniteTurnStop(struct mg_connection *c, mg_http_message *hm) {
    user_input = STOP_INFINITE;
    Serial.println("turn_mode = STOP");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSpeedInfinitePlus(struct mg_connection *c, mg_http_message *hm) {
    infinite_speed = infinite_speed + 20;
    Serial.println("Infinite speed increased");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSpeedInfiniteMinus(struct mg_connection *c, mg_http_message *hm) {
    infinite_speed = infinite_speed - 20;
    Serial.println("Infinite speed decreased");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleOpenSettings(struct mg_connection *c, struct mg_http_message *hm) {
    user_input = SETTINGS;
    Serial.println("open Settings: user_input = SETTINGS");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleCloseSettings(struct mg_connection *c, struct mg_http_message *hm) {
    user_input = STOP_INFINITE;
    Serial.println("close Settings: user_input = STOP_INFINITE");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleArrowUp(struct mg_connection *c, struct mg_http_message *hm) {
    digitalWrite(ENA_PIN, LOW); // Enable the driver motor
    stepper1.setRampLen(3);
    stepper1.move(15);
    stepper1.setRampLen(0);
    digitalWrite(ENA_PIN, HIGH); // Disable the driver motor
    Serial.println("Arrow up button pressed");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleArrowDown(struct mg_connection *c, struct mg_http_message *hm) {
    digitalWrite(ENA_PIN, LOW); // Enable the driver motor
    stepper1.setRampLen(3);
    stepper1.move(-15);
    stepper1.setRampLen(0);
    digitalWrite(ENA_PIN, HIGH); // Disable the driver motor
    Serial.println("Arrow down button pressed");
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSubmitFrontPos(struct mg_connection *c, struct mg_http_message *hm) {
    front_pos = stepper1.currentPosition();
    front_pos_defined = true;
    Serial.println("front_pos submitted at: " + String(front_pos));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSubmitRearPos(struct mg_connection *c, struct mg_http_message *hm) {
    front_pos = front_pos + std::abs(stepper1.currentPosition());
    stepper1.set_Zero();
    rear_pos_defined = true;
    Serial.println("rear_pos submitted at: " + String(0) + " front_pos: " + String(front_pos));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSetSingleSpeed(struct mg_connection *c, struct mg_http_message *hm) {
    char param[256];
    // Extract the 'value' parameter from the query string
    if (mg_http_get_var(&hm->query, "value", param, sizeof(param)) <= 0) {
        Serial.println("Missing 'value' parameter");
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'value' parameter");
        return;
    }
    single_speed = speed_multi * atof(param);
    Serial.println("New single Speed: " + String(single_speed));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSetInfiniteSpeed(struct mg_connection *c, struct mg_http_message *hm) {
    char param[256];
    // Extract the 'value' parameter from the query string
    if (mg_http_get_var(&hm->query, "value", param, sizeof(param)) <= 0) {
        Serial.println("Missing 'value' parameter");
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'value' parameter");
        return;
    }
    infinite_speed = speed_multi * atof(param);
    Serial.println("New infinite Speed: " + String(infinite_speed));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
}

void handleSendCofig(struct mg_connection *c, struct mg_http_message *hm) {
    char param[128];
    char nssid[128];
    char npassword[128];
    if (mg_http_get_var(&hm->body, "ssid", param, sizeof(param)) <= 0) {
        Serial.println("Missing 'ssid' parameter");
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'ssid' parameter");
        return;
    }
    strcpy(nssid, param);
    if (mg_http_get_var(&hm->body, "password", param, sizeof(param)) <= 0) {
        Serial.println("Missing 'password' parameter");
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'password' parameter");
        return;
    }
    strcpy(npassword, param);

    Serial.println(nssid);
    Serial.println(npassword);

    // Connect to the new Wi-Fi network
    // search for ssid
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan done.");
    bool ssidFound = false;
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        for (int i = 0; i < n; ++i) {
            Serial.println("New SSID found: " + String(WiFi.SSID(i)));
            if (WiFi.SSID(i) == String(nssid)) {
                ssidFound = true;
                break;
            }
        }
    }

    // Attempt to connect to the stored SSID
    if (ssidFound) {
        WiFi.begin(nssid, npassword);

        // Wait for connection
        unsigned long startTime = millis();
        const unsigned long timeout = 10000; // 10 seconds timeout
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime >= timeout) {
                Serial.println("Failed to connect to WiFi: Timeout");
                mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Failed to connect to WiFi: Timeout");
                return;
            }
            delay(1000);
            Serial.println("Connecting to WiFi...");
        }
        Serial.println("Connected to WiFi");

        // Save the new WiFi credentials to the wifi.txt file
        if (saveCredentials(String(nssid), String(npassword))) {
            Serial.println("WiFi credentials saved");
        } else {
            Serial.println("Failed to save WiFi credentials");
        }

        //************* Re-Init Web Server *******************
        // starting http server
        String httpUrl = "http://" + WiFi.localIP().toString() + ":" + String(HTTP_PORT);
        Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
        mg_http_listen(&mgr, httpUrl.c_str(), httpEventHandler, NULL);

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
        String ipAddress = WiFi.localIP().toString();
        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", ipAddress.c_str());
    } else {
        Serial.println("Target SSID not found");
        mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Failed to connect to WiFi: Target SSID not found");
    }
}

void handleWifiState(struct mg_connection *c, struct mg_http_message *hm) {
    String response = "{\"status\":\"";
    if (WiFi.status() == WL_CONNECTED) {
        response += "connected\",\"ssid\":\"" + WiFi.SSID() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    } else {
        response += "disconnected\"}";
    }
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", response.c_str());
}

void handleSpeeds(struct mg_connection *c, struct mg_http_message *hm) {
    String response = "{\"single_speed\":\"" + String(single_speed) + 
                    "\",\"max_single_speed\":\"" + String(max_single_speed) + 
                    "\",\"infinite_speed\":\"" + String(infinite_speed) + 
                    "\",\"max_infinite_speed\":\"" + String(max_infinite_speed) + "\"}";
                    
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", response.c_str());
}

// get the number of active connections
static inline int numconns(struct mg_mgr *mgr) {
    int n = 0;
    for (struct mg_connection *t = mgr->conns; t != NULL; t = t->next)
        n++;
    return n;
}

// dns event handler
static void dnsEventHandler(struct mg_connection *c, int ev, void *ev_data) {
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
    }
    (void)ev_data;
}

// http event handlers
static void httpEventHandler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/automaticModeOn"), NULL)) {
            handleAutomaticModeOn(c, hm);
        } else if (mg_match(hm->uri, mg_str("/automaticModeOff"), NULL)) {
            handleAutomaticModeOff(c, hm);
        } else if (mg_match(hm->uri, mg_str("/half"), NULL)) {
            handleHalf(c, hm);
        } else if (mg_match(hm->uri, mg_str("/full"), NULL)) {
            handleFull(c, hm);
        } else if (mg_match(hm->uri, mg_str("/single_turn_start"), NULL)) {
            handleSingleTurnStart(c, hm);
        } else if (mg_match(hm->uri, mg_str("/single_speed_plus"), NULL)) {
            handleSingleSpeedPlus(c, hm);
        } else if (mg_match(hm->uri, mg_str("/single_speed_minus"), NULL)) {
            handleSingleSpeedMinus(c, hm);
        } else if (mg_match(hm->uri, mg_str("/infinite_turn_start"), NULL)) {
            handleInfiniteTurnStart(c, hm);
        } else if (mg_match(hm->uri, mg_str("/infinite_turn_stop"), NULL)) {
            handleInfiniteTurnStop(c, hm);
        } else if (mg_match(hm->uri, mg_str("/speed_infinite_plus"), NULL)) {
            handleSpeedInfinitePlus(c, hm);
        } else if (mg_match(hm->uri, mg_str("/speed_infinite_minus"), NULL)) {
            handleSpeedInfiniteMinus(c, hm);
        }
        // *********** SETTINGS ***********************************
        else if (mg_match(hm->uri, mg_str("/openSettings"), NULL)) {
            handleOpenSettings(c, hm);
        } else if (mg_match(hm->uri, mg_str("/closeSettings"), NULL)) {
            handleCloseSettings(c, hm);
        } else if (mg_match(hm->uri, mg_str("/arrowUp"), NULL)) {
            handleArrowUp(c, hm);
        } else if (mg_match(hm->uri, mg_str("/arrowDown"), NULL)) {
            handleArrowDown(c, hm);
        } else if (mg_match(hm->uri, mg_str("/submitFrontPos"), NULL)) {
            handleSubmitFrontPos(c, hm);
        } else if (mg_match(hm->uri, mg_str("/submitRearPos"), NULL)) {
            handleSubmitRearPos(c, hm);
        } else if (mg_match(hm->uri, mg_str("/setSingleSpeed"), NULL)) {
            handleSetSingleSpeed(c, hm);
        } else if (mg_match(hm->uri, mg_str("/setInfiniteSpeed"), NULL)) {
            handleSetInfiniteSpeed(c, hm);
        } else if (mg_match(hm->uri, mg_str("/hotspot-detect.html"), NULL)) {
            mg_http_serve_file(c, hm, "/captive_page.html", &s_http_serve_opts);
        } else if (mg_match(hm->uri, mg_str("/generate_204"), NULL)) {
            mg_http_serve_file(c, hm, "/captive_page.html", &s_http_serve_opts);
        } else if (mg_match(hm->uri, mg_str("/gen_204"), NULL)) {
            mg_http_serve_file(c, hm, "/captive_page.html", &s_http_serve_opts);
        } else if (mg_match(hm->uri, mg_str("/send-config"), NULL)) {
            handleSendCofig(c, hm);
        } else if (mg_match(hm->uri, mg_str("/wifi-state"), NULL)) {
            handleWifiState(c, hm);
        } else if (mg_match(hm->uri, mg_str("/speeds"), NULL)) {
            handleSpeeds(c, hm);
        } else {
            // by default serve static files
            mg_http_serve_dir(c, hm, &s_http_serve_opts);
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
        if (numconns(c->mgr) > 20) {
            MG_ERROR(("Too many connections"));
            c->is_closing = 1;
        }
    } else if (ev == MG_EV_ERROR) {
        // on error flush the mg_io_buff
        c->is_closing = 1;
    }
}

void networkTask(void *pvParameters) {
    //************* Web Server *******************
    // initialize mongoose event manager
    mg_mgr_init(&mgr);

    // initialize mongoose static filesystem
    s_http_serve_opts = {
        .root_dir = "/",
        .ssi_pattern = "",
        .extra_headers = "",
        .mime_types = "",
        .page404 = NULL,
        .fs = &littlefs_interface,
    };

    connectToWiFi();

    // starting http server in access point mode
    String httpUrl = "http://" + WiFi.softAPIP().toString() + ":" + String(HTTP_PORT);
    Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
    mg_http_listen(&mgr, httpUrl.c_str(), httpEventHandler, NULL);

    String dnsURL = "udp://" + WiFi.softAPIP().toString() + ":" + String(DNS_PORT);
    Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting DNS listener on " + dnsURL);
    mg_listen(&mgr, dnsURL.c_str(), dnsEventHandler, NULL);

    // Set Root CA certificate
    client.setCACert(ca_cert);

    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqttEventHandler);

    while (true) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!mqtt_client.connected()) {
                connectToMQTT();
            }
            mqtt_client.loop();
        }

        mg_mgr_poll(&mgr, 1000);
    }
}
#pragma endregion

/**
 * @brief prints the current location of the motor evertime it changes
 */
void print_stepper_position() {
    static int last_motor_pos = stepper1.currentPosition();

    if (stepper1.currentPosition() != last_motor_pos) {
        Serial.println("Mot Pos: " + String(stepper1.currentPosition()));
        last_motor_pos = stepper1.currentPosition();
    }
}

/**
 * @brief prints the current location of the motor evertime it changes
 */
void print_single_speed() {
    if (single_speed != last_single_speed) {
        Serial.println("single speed: " + String(single_speed));
        last_single_speed = single_speed;
    }
}

/**
 * @brief prints the current location of the motor evertime it changes
 */
void print_infinite_speed() {
    if (infinite_speed != last_infinite_speed) {
        Serial.println("single speed: " + String(infinite_speed));
        last_infinite_speed = infinite_speed;
    }
}

void setup() {
    io_setup();

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(19, OUTPUT);
    stepper1.set_Zero();
    stepper1.setSpeed(CALIBRATION_SPEED);

    Serial.begin(115200);

    // Setting up littlefs File System, required for HTML and dataloggin
    if (!LittleFS.begin()) { // Mounts the littlefs file system and handle littlefs
        // Errors:I
        Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "An Error has occurred while mounting SPIFFS");
        return;
    }

    // List all files in littlefs (for debugging purposes)
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }

    // ***** WIFI SETUP *****
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid);
    Serial.println("Access Point Started");
    Serial.println(WiFi.softAPIP());

    xTaskCreatePinnedToCore(networkTask, "networkTask", 50000, NULL, 0, NULL, 0);
}

void connectToWiFi() {
    // Check for stored SSID and password
    String ssid, password;
    if (readCredentials(ssid, password)) {
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
            Serial.print("Connecting to WiFi");
            int counter = 0;
            while (WiFi.status() != WL_CONNECTED && counter < 20) {
                delay(500);
                Serial.print(".");
                counter++;
            }
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Connected to " + ssid);
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());
                // starting http server
                String httpUrl = "http://" + WiFi.localIP().toString() + ":" + String(HTTP_PORT);
                Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
                mg_http_listen(&mgr, httpUrl.c_str(), httpEventHandler, NULL);

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
    } else {
        Serial.println("No stored WiFi credentials found");
    }
}

void connectToMQTT() {
    while (!mqtt_client.connected()) {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(motor_state_topic);
            mqtt_client.subscribe(motor_speed_topic);
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" Retrying in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttEventHandler(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);

    // Convert the payload into a JSON object
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
    }

    Serial.print("Message: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println("\n-----------------------");

    // Process the JSON based on the topic
    if (String(topic) == "esp32/motorState") {
        // Assuming the payload has a key "state" for motorState topic
        if (doc.containsKey("state")) {
            const char *state = doc["state"];
            Serial.print("Motor State: ");
            Serial.println(state);
            if (String(state) == "START_SINGLE") {
                user_input = START_SINGLE;
            } else if (String(state) == "START_INFINITE") {
                user_input = START_INFINITE;
            } else if (String(state) == "STOP_INFINITE") {
                user_input = STOP_INFINITE;
            }
        }
    } else if (String(topic) == "esp32/motorSpeed") {
        // Assuming the payload has a key "type" and "speed" for motorSpeed topic
        if (!doc.containsKey("type")) {
            return;
        }
        if (!doc.containsKey("speed")) {
            return;
        }

        const char *type = doc["type"];
        int speed = doc["speed"];

        Serial.println(String(type) + ": " + speed);
        if (String(type) == "SINGLE_SPEED") {
            single_speed = speed;
        } else if (String(type) == "INFINITE_SPEED") {
            infinite_speed = speed;
        }
    }
}

int distance_to_go = 0;
int half_ref_delay = 1000; // how long the motor should wait after a half revolution in ms
int position_to_travel = 0;
long stop_time = 0;
long start_single_pause = 0;
long start_single_pause_back = 0;
bool turn_finished = true; // to check if the turn is finish and a new can lanched

void loop() {
    // print_stepper_position();
    // print_single_speed();
    // print_infinite_speed();

    if (automatic_mode) {
        // nextTURNTime(millis());
    }

    currentMillis = millis();
    // work with user_input
    if (user_input == STOP_INFINITE) {
        if (turn_finished) {
            turn_mode = STOP;
            motor_state = SINGLE_START;
        }
    }

    if (user_input == START_SINGLE) {
        if (front_pos_defined && rear_pos_defined) {
            turn_mode = SINGLE;
        } else {
            Serial.println("define pos firs");
            user_input = STOP_INFINITE;
        }
    }

    if (user_input == START_INFINITE) {
        if (front_pos_defined && rear_pos_defined) {
            turn_mode = INFINITE;
        } else {
            Serial.println("define pos firs");
            user_input = STOP_INFINITE;
        }
    }

    if (stepper1.distanceToGo()) {
        digitalWrite(ENA_PIN, LOW); // Enable the driver motor
    } else {
        digitalWrite(ENA_PIN, HIGH); // Disable the driver motor
    }

// ***************SINGLE TURN MODE******************
#pragma region SINGLE
    // if Single button is pressed, start the movement
    if (turn_mode == SINGLE) {
        turn_finished = false;
        user_input = STOP_INFINITE;
        if (motor_state == SINGLE_START) {
            stepper1.setSpeed(single_speed);
            if (tool_type == HALFREV) {
                stepper1.moveTo(front_pos);
                motor_state = SINGLE_ROTATE;
                Serial.println("180");
            }
            if (tool_type == FULLREV) {
                stepper1.moveTo(200);
                motor_state = SINGLE_ROTATE;
                Serial.println("360");
            }
        }
        // loops in this mode as long as the stepper is moving --> starts to count
        // for the break
        if (motor_state == SINGLE_ROTATE) {
            stepper1.setSpeed(single_speed);
            if (stepper1.distanceToGo() == 0) {
                motor_state = SINGLE_PAUSE;
                start_single_pause = currentMillis;
                Serial.println("Pause");
            }
        }
        // Serial.println(turn_mode);

        if (motor_state == SINGLE_PAUSE) {
            long pauseT;
            if (tool_type == HALFREV) {
                pauseT = 1000;
            }
            if (tool_type == FULLREV) {
                pauseT = 300;
            }
            if (currentMillis > start_single_pause + pauseT) {
                motor_state = ROTATE_BACK;
                Serial.println("Pause ende");
            }
        }

        // rotate Back
        if (motor_state == ROTATE_BACK) {
            if (tool_type == HALFREV) {
                stepper1.setRampLen(10);
                stepper1.setSpeed(int(90));
                stepper1.moveTo(rear_pos);
                // Serial.println(stepper1.currentPosition());
                if (stepper1.distanceToGo() == 0) {
                    turn_mode = STOP;
                    Serial.println("Back Position reached");
                    turn_finished = true;
                    stepper1.setRampLen(0);
                    stepper1.stop();
                }
            }
            if (tool_type == FULLREV) { // In Fullrev mode the zero pos is reset
                                        //--> otherwise the
                                        // position will sum up over every rotation
                turn_mode = STOP;
                stepper1.set_Zero();
                turn_finished = true;
                stepper1.stop();
            }
        }
    }
#pragma endregion

// ***************INFINITE TURN MODE****************
#pragma region INFINITE

    if (turn_mode == INFINITE) {
        turn_finished = false;

        if (tool_type == FULLREV) {
            stepper1.setSpeed(infinite_speed);
            stepper1.rotate(1);
            if (stepper1.currentPosition() == 200) {
                stepper1.set_Zero();
                turn_finished = true;
            }
        }
        // to stop the infinite turn
        if (turn_finished == true && user_input == STOP_INFINITE) {
            stepper1.stop();
            stepper1.moveTo(0);
        }

        if (tool_type == HALFREV) {
            if (motor_state == SINGLE_START) {
                stepper1.setSpeed(infinite_speed);
                stepper1.moveTo(front_pos);
                motor_state = SINGLE_ROTATE;
                Serial.println("180");
            }

            // loops in this mode as long as the stepper is moving --> starts to
            // count for the break
            if (motor_state == SINGLE_ROTATE) {
                // Serial.println(stepper1.currentPosition());
                stepper1.setSpeed(infinite_speed);
                if (stepper1.distanceToGo() == 0) {
                    motor_state = SINGLE_PAUSE;
                    start_single_pause = currentMillis;
                    Serial.println("Pause");
                    stepper1.stop();
                }
            }

            // break mode
            if (motor_state == SINGLE_PAUSE) {
                if (currentMillis > start_single_pause + 1000) {
                    motor_state = ROTATE_BACK;
                    Serial.println("Pause ende");
                }
            }

            // rotate Back
            if (motor_state == ROTATE_BACK) {
                stepper1.setRampLen(10);
                stepper1.setSpeed(int(90));
                stepper1.moveTo(rear_pos);

                if (stepper1.distanceToGo() == 0) {
                    motor_state = SINGLE_PAUSE_BACK;
                    start_single_pause_back = millis();
                    Serial.println("Back Position reached");
                    stepper1.setRampLen(0);
                    stepper1.stop();
                }
            }

            // break mode
            if (motor_state == SINGLE_PAUSE_BACK) {
                if (currentMillis > start_single_pause_back + 1000) {
                    motor_state = SINGLE_START;
                    Serial.println("Pause back ende");
                    turn_finished = true;
                }
            }
        }
    }
#pragma endregion
}
