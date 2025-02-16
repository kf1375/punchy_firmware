#ifndef WEB_UPDATER_H
#define WEB_UPDATER_H

#include "configuration.h"
#include "logging.h"
#include <esp32FOTA.hpp>

//************************************************************************************************
// Class for Firmware updates. Will check for new firmware when internet
// connection is available Will start update if triggered updateStart in UI
// ***********************************************************************************************

class WebUpdater
{
public:
  WebUpdater(Configuration &config) : m_config(config) {}
  ~WebUpdater() {}
  void setup();
  void loop();

private:
  Configuration &m_config;

  const String m_deviceName = "testiwhisk-device";
  String m_deviceVersion;
  esp32FOTA *m_esp32FOTA;
  void setUpdatedVersion();
  void updateFinishedCallback(int partition, bool restart_after);

  const char *m_manifestUrl =
      "https://myremotedevice.com/api/update/manifest.json";
};
#endif // WEB_UPDATER_H