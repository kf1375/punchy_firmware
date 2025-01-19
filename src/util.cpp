#include "util.h"

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

void Util::printMgStr(const mg_str &str)
{
    for (size_t i = 0; i < str.len; i++) {
        Serial.print(str.buf[i]);
    }
    Serial.print("\n");
}