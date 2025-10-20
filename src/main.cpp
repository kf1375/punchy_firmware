#include <Arduino.h>

#include "configuration.h"
#include "hardware_controller.h"
#include "logging.h"
#include "network_manager.h"

Configuration config;
HardwareController hardwareController(config);
NetworkManager networkManager(config, hardwareController);

void networkTask(void *pvParameters)
{
  networkManager.setup();
  while (true) {
    networkManager.loop();
  }
}

void setup()
{
  Serial.begin(115200);

  config.begin();
  hardwareController.begin();

  xTaskCreatePinnedToCore(networkTask, "networkTask", 50000, NULL, 0, NULL, 0);
}

void loop()
{
  hardwareController.loop();
  if (hardwareController.state() == HardwareController::State::Idle) {
    config.loop();
  }
}
