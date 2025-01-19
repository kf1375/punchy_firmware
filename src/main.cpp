// #include "custom_functions.h"
// #include "driver.h"
// #include "fs_interface.h"
// #include "global_variables.h"
// #include "io.h"
#include <Arduino.h>

#include "network_manager.h"
#include "hardware_controller.h"

// #include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
// #include <PubSubClient.h>
// #include <WiFi.h>
// #include <WiFiClientSecure.h>

// **********CREATE OBJECTS**********
// For stepper motor
// driver stepper1;

// WiFi and MQTT client initialization
// WiFiClientSecure client;
// PubSubClient mqtt_client(client);

// **********ENUMS**********
// enum toolType { FULLREV, HALFREV };
// toolType tool_type = FULLREV;

// enum turnMode { STOP, SINGLE, INFINITE };
// turnMode turn_mode = STOP;

// enum userInput { START_SINGLE, START_INFINITE, STOP_INFINITE, SETTINGS }; // in Json user_mode
// userInput user_input = STOP_INFINITE;

// enum motorState {
//     SINGLE_START,  // starts the single turn
//     SINGLE_ROTATE, // durning forward movement
//     SINGLE_PAUSE,  // pause at hit position
//     SINGLE_PAUSE_BACK,
//     ROTATE_BACK, // moving from hit to home pos

//     INFINITE_ROTATE, // starts the stepper1.rotate
//     INFINITE_BACK    // brings the tool to the home pos after the rotate
// };
// motorState motor_state = SINGLE_START;

CommandQueue commandQueue;

void networkTask(void *pvParameters) 
{
    NetworkManager networkManager;
    networkManager.init();
    networkManager.setCommandQueue(&commandQueue);
    // networkManager.enableHotspot("HELLO");
    // networkManager.startDnsServer();
    // networkManager.startHttpServer();
    networkManager.connectWifi("MyEden", "Kazem1375");
    networkManager.connectMqttBroker();
    // // Set Root CA certificate
    // client.setCACert(ca_cert);

    // mqtt_client.setServer(mqtt_broker, mqtt_port);
    // mqtt_client.setKeepAlive(60);
    // mqtt_client.setCallback(mqttEventHandler);

    while (true) {
        networkManager.poll();
    }
}

void hardwareTask() {
    HardwareController hardwareController;
    hardwareController.init();
    hardwareController.setCommandQueue(&commandQueue);

    while (true)
    {
        hardwareController.poll();
    }
    
}

void setup() {
    // io_setup();

    // // initialize digital pin LED_BUILTIN as an output.
    // pinMode(19, OUTPUT);
    // stepper1.set_Zero();
    // stepper1.setSpeed(CALIBRATION_SPEED);

    Serial.begin(115200);

    // Setting up littlefs File System, required for HTML and dataloggin
    if (!LittleFS.begin()) { // Mounts the littlefs file system and handle littlefs
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

    xTaskCreatePinnedToCore(networkTask, "networkTask", 50000, NULL, 0, NULL, 0);
}

// void connectToWiFi() {
//     // Check for stored SSID and password
//     String ssid, password;
//     if (readCredentials(ssid, password)) {
//         // search for ssid
//         Serial.println("Scanning for WiFi networks...");
//         WiFi.disconnect(true);
//         int n = WiFi.scanNetworks();
//         Serial.println("Scan done.");
//         bool ssidFound = false;
//         if (n == 0) {
//             Serial.println("No networks found");
//         } else {
//             for (int i = 0; i < n; ++i) {
//                 if (WiFi.SSID(i) == ssid) {
//                     ssidFound = true;
//                     break;
//                 }
//             }
//         }

//         // Attempt to connect to the stored SSID
//         if (ssidFound) {
//             WiFi.begin(ssid.c_str(), password.c_str());
//             Serial.print("Connecting to WiFi");
//             int counter = 0;
//             while (WiFi.status() != WL_CONNECTED && counter < 20) {
//                 delay(500);
//                 Serial.print(".");
//                 counter++;
//             }
//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("Connected to " + ssid);
//                 Serial.print("IP Address: ");
//                 Serial.println(WiFi.localIP());
//                 // starting http server
//                 String httpUrl = "http://" + WiFi.localIP().toString() + ":" + String(HTTP_PORT);
//                 Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " + "Starting HTTP listener on " + httpUrl);
//                 mg_http_listen(&m_mgr, httpUrl.c_str(), httpEventHandler, NULL);

//                 // starting https server
//                 // String httpsUrl = "https://" + WiFi.localIP().toString() + ":" + String(HTTPS_PORT);
//                 // Serial.println(String(__FILE__) + ":" + String(__LINE__) + ": " +
//                 //                "Starting HTTPS listener on " + httpsUrl);
//                 // mg_http_listen(&mgr, httpsUrl.c_str(), httpEventHandler, NULL);

//                 // starting http websocket
//                 // String socketUrl = "http://" + WiFi.localIP().toString() + ":" + String(WS_PORT);
//                 // mg_http_listen(&mgr, socketUrl.c_str(), WebSocket::event_handler, NULL);

//                 // starting https websocket
//                 // String secureSockteUrl = "https://" + WiFi.localIP().toString() + ":" + String(WSS_PORT);
//                 // mg_http_listen(&mgr, secureSockteUrl.c_str(), WebSocket::event_handler, NULL);

//                 // Serial.println(String(__FILE__) + ":" + String(__LINE__) +
//                 //                ": Starting WS listener on " + socketUrl);
//             } else {
//                 Serial.println("Failed to connect to WiFi");
//             }
//         } else {
//             Serial.println("Target SSID not found");
//         }
//     } else {
//         Serial.println("No stored WiFi credentials found");
//     }
// }

// void connectToMQTT() {
//     while (!mqtt_client.connected()) {
//         String client_id = "esp32-client-" + String(WiFi.macAddress());
//         Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
//         if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
//             Serial.println("Connected to MQTT broker");
//             mqtt_client.subscribe(motor_state_topic);
//             mqtt_client.subscribe(motor_speed_topic);
//         } else {
//             Serial.print("Failed to connect to MQTT broker, rc=");
//             Serial.print(mqtt_client.state());
//             Serial.println(" Retrying in 5 seconds.");
//             delay(5000);
//         }
//     }
// }

// void mqttEventHandler(char *topic, byte *payload, unsigned int length) {
//     Serial.print("Message received on topic: ");
//     Serial.println(topic);

//     // Convert the payload into a JSON object
//     JsonDocument doc;
//     DeserializationError error = deserializeJson(doc, payload, length);

//     if (error) {
//         Serial.print("deserializeJson() failed: ");
//         Serial.println(error.f_str());
//         return;
//     }

//     Serial.print("Message: ");
//     for (unsigned int i = 0; i < length; i++) {
//         Serial.print((char)payload[i]);
//     }
//     Serial.println("\n-----------------------");

//     // Process the JSON based on the topic
//     if (String(topic) == "esp32/motorState") {
//         // Assuming the payload has a key "state" for motorState topic
//         if (doc.containsKey("state")) {
//             const char *state = doc["state"];
//             Serial.print("Motor State: ");
//             Serial.println(state);
//             if (String(state) == "START_SINGLE") {
//                 user_input = START_SINGLE;
//             } else if (String(state) == "START_INFINITE") {
//                 user_input = START_INFINITE;
//             } else if (String(state) == "STOP_INFINITE") {
//                 user_input = STOP_INFINITE;
//             }
//         }
//     } else if (String(topic) == "esp32/motorSpeed") {
//         // Assuming the payload has a key "type" and "speed" for motorSpeed topic
//         if (!doc.containsKey("type")) {
//             return;
//         }
//         if (!doc.containsKey("speed")) {
//             return;
//         }

//         const char *type = doc["type"];
//         int speed = doc["speed"];

//         Serial.println(String(type) + ": " + speed);
//         if (String(type) == "SINGLE_SPEED") {
//             single_speed = speed;
//         } else if (String(type) == "INFINITE_SPEED") {
//             infinite_speed = speed;
//         }
//     }
// }

// int distance_to_go = 0;
// int half_ref_delay = 1000; // how long the motor should wait after a half revolution in ms
// int position_to_travel = 0;
// long stop_time = 0;
// long start_single_pause = 0;
// long start_single_pause_back = 0;
// bool turn_finished = true; // to check if the turn is finish and a new can lanched

void loop() {
    hardwareTask();
    // print_stepper_position();
    // print_single_speed();
    // print_infinite_speed();

//     if (automatic_mode) {
//         // nextTURNTime(millis());
//     }

//     currentMillis = millis();
//     // work with user_input
//     if (user_input == STOP_INFINITE) {
//         if (turn_finished) {
//             turn_mode = STOP;
//             motor_state = SINGLE_START;
//         }
//     }

//     if (user_input == START_SINGLE) {
//         if (front_pos_defined && rear_pos_defined) {
//             turn_mode = SINGLE;
//         } else {
//             Serial.println("define pos firs");
//             user_input = STOP_INFINITE;
//         }
//     }

//     if (user_input == START_INFINITE) {
//         if (front_pos_defined && rear_pos_defined) {
//             turn_mode = INFINITE;
//         } else {
//             Serial.println("define pos firs");
//             user_input = STOP_INFINITE;
//         }
//     }

//     if (stepper1.distanceToGo()) {
//         digitalWrite(ENA_PIN, LOW); // Enable the driver motor
//     } else {
//         digitalWrite(ENA_PIN, HIGH); // Disable the driver motor
//     }

// // ***************SINGLE TURN MODE******************
// #pragma region SINGLE
//     // if Single button is pressed, start the movement
//     if (turn_mode == SINGLE) {
//         turn_finished = false;
//         user_input = STOP_INFINITE;
//         if (motor_state == SINGLE_START) {
//             stepper1.setSpeed(single_speed);
//             if (tool_type == HALFREV) {
//                 stepper1.moveTo(front_pos);
//                 motor_state = SINGLE_ROTATE;
//                 Serial.println("180");
//             }
//             if (tool_type == FULLREV) {
//                 stepper1.moveTo(200);
//                 motor_state = SINGLE_ROTATE;
//                 Serial.println("360");
//             }
//         }
//         // loops in this mode as long as the stepper is moving --> starts to count
//         // for the break
//         if (motor_state == SINGLE_ROTATE) {
//             stepper1.setSpeed(single_speed);
//             if (stepper1.distanceToGo() == 0) {
//                 motor_state = SINGLE_PAUSE;
//                 start_single_pause = currentMillis;
//                 Serial.println("Pause");
//             }
//         }
//         // Serial.println(turn_mode);

//         if (motor_state == SINGLE_PAUSE) {
//             long pauseT;
//             if (tool_type == HALFREV) {
//                 pauseT = 1000;
//             }
//             if (tool_type == FULLREV) {
//                 pauseT = 300;
//             }
//             if (currentMillis > start_single_pause + pauseT) {
//                 motor_state = ROTATE_BACK;
//                 Serial.println("Pause ende");
//             }
//         }

//         // rotate Back
//         if (motor_state == ROTATE_BACK) {
//             if (tool_type == HALFREV) {
//                 stepper1.setRampLen(10);
//                 stepper1.setSpeed(int(90));
//                 stepper1.moveTo(rear_pos);
//                 // Serial.println(stepper1.currentPosition());
//                 if (stepper1.distanceToGo() == 0) {
//                     turn_mode = STOP;
//                     Serial.println("Back Position reached");
//                     turn_finished = true;
//                     stepper1.setRampLen(0);
//                     stepper1.stop();
//                 }
//             }
//             if (tool_type == FULLREV) { // In Fullrev mode the zero pos is reset
//                                         //--> otherwise the
//                                         // position will sum up over every rotation
//                 turn_mode = STOP;
//                 stepper1.set_Zero();
//                 turn_finished = true;
//                 stepper1.stop();
//             }
//         }
//     }
// #pragma endregion

// // ***************INFINITE TURN MODE****************
// #pragma region INFINITE

//     if (turn_mode == INFINITE) {
//         turn_finished = false;

//         if (tool_type == FULLREV) {
//             stepper1.setSpeed(infinite_speed);
//             stepper1.rotate(1);
//             if (stepper1.currentPosition() == 200) {
//                 stepper1.set_Zero();
//                 turn_finished = true;
//             }
//         }
//         // to stop the infinite turn
//         if (turn_finished == true && user_input == STOP_INFINITE) {
//             stepper1.stop();
//             stepper1.moveTo(0);
//         }

//         if (tool_type == HALFREV) {
//             if (motor_state == SINGLE_START) {
//                 stepper1.setSpeed(infinite_speed);
//                 stepper1.moveTo(front_pos);
//                 motor_state = SINGLE_ROTATE;
//                 Serial.println("180");
//             }

//             // loops in this mode as long as the stepper is moving --> starts to
//             // count for the break
//             if (motor_state == SINGLE_ROTATE) {
//                 // Serial.println(stepper1.currentPosition());
//                 stepper1.setSpeed(infinite_speed);
//                 if (stepper1.distanceToGo() == 0) {
//                     motor_state = SINGLE_PAUSE;
//                     start_single_pause = currentMillis;
//                     Serial.println("Pause");
//                     stepper1.stop();
//                 }
//             }

//             // break mode
//             if (motor_state == SINGLE_PAUSE) {
//                 if (currentMillis > start_single_pause + 1000) {
//                     motor_state = ROTATE_BACK;
//                     Serial.println("Pause ende");
//                 }
//             }

//             // rotate Back
//             if (motor_state == ROTATE_BACK) {
//                 stepper1.setRampLen(10);
//                 stepper1.setSpeed(int(90));
//                 stepper1.moveTo(rear_pos);

//                 if (stepper1.distanceToGo() == 0) {
//                     motor_state = SINGLE_PAUSE_BACK;
//                     start_single_pause_back = millis();
//                     Serial.println("Back Position reached");
//                     stepper1.setRampLen(0);
//                     stepper1.stop();
//                 }
//             }

//             // break mode
//             if (motor_state == SINGLE_PAUSE_BACK) {
//                 if (currentMillis > start_single_pause_back + 1000) {
//                     motor_state = SINGLE_START;
//                     Serial.println("Pause back ende");
//                     turn_finished = true;
//                 }
//             }
//         }
//     }
// #pragma endregion
}
