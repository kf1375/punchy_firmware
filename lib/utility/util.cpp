#include "util.h"

#include <FS.h>
#include <LittleFS.h>

String Util::getMacAddress()
{
  uint8_t device_mac_address[6];
  esp_efuse_mac_get_default(device_mac_address);

  String device_id = "";
  for (uint8_t i = 0; i < 6; i++) {
    device_id += String(device_mac_address[i], HEX);
  }
  device_id.toUpperCase();
  return device_id;
}

int Util::getNumberOfConnections(struct mg_mgr *mgr)
{
  int n = 0;
  for (struct mg_connection *t = mgr->conns; t != NULL; t = t->next)
    n++;
  return n;
}

bool Util::timeElapsed(float seconds, long startT)
{
  float elapsedSeconds = (millis() - startT) / 1000.0;

  if (elapsedSeconds > seconds)
    return true;

  return false;
}