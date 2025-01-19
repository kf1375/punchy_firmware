// #include "custom_functions.h"
// #include "driver.h"
// #include "fs_interface.h"
// #include "global_variables.h"
// #include "io.h"
#include <Arduino.h>

#include "global_variables.h"
#include "network_manager.h"
#include "hardware_controller.h"

#include <FS.h>
#include <LittleFS.h>

CommandQueue commandQueue;

void networkTask(void *pvParameters) 
{
    NetworkManager networkManager;
    networkManager.init();
    networkManager.setCommandQueue(&commandQueue);
    networkManager.connectWifi(ssid, password);
    networkManager.connectMqttBroker();

    while (true) {
        networkManager.poll();
    }
}

void hardwareTask() {
    HardwareController hardwareController(STEP_PIN, DIR_PIN, ENA_PIN);
    hardwareController.init();
    hardwareController.setCommandQueue(&commandQueue);

    while (true) {
        hardwareController.poll();
    }
    
}

void setup() {
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

void loop() {
    hardwareTask();
}
