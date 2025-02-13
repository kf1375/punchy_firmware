#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "mongoose.h"
#include "mongoose_config.h"

#include "configuration.h"
#include "hardware_controller.h"

class WebServer
{
public:
  WebServer(mg_mgr &mgr, Configuration &config,
            HardwareController &hwController);

  void setup();
  void loop();

private:
  Configuration &m_config;
  struct mg_mgr &m_mgr;
  HardwareController &m_hwController;
  struct mg_connection *m_conn;

  const struct mg_http_serve_opts m_httpServeOpts = {
      .root_dir = "/littlefs/static",
      .extra_headers = "Connection: close\r\n",
      .page404 = "/littlefs/static/captive_page.html",
  };

  void stop();

  void eventHandler(struct mg_connection *c, int ev, void *ev_data);
  // Endpoints
  void handleSetWifi(struct mg_connection *c, struct mg_http_message *hm);
  void handleGetWifi(struct mg_connection *c, struct mg_http_message *hm);

  void sendJsonResponse(struct mg_connection *c, JsonObject &output);
  void sendJsonResponse(struct mg_connection *c, JsonDocument &output);
  void sendJsonResponse(struct mg_connection *c, JsonArray &output);
  void sendTextResponse(struct mg_connection *c, size_t code, String text);
  void sendTextResponse(struct mg_connection *c, size_t code, const char *text);

  String extractStringVariable(struct mg_http_message *hm, String variable);
  bool extractBoolVariable(struct mg_http_message *hm, String variable);
  float extractFloatVariable(struct mg_http_message *hm, String variable);
  int extractIntVariable(struct mg_http_message *hm, String variable);
};

#endif // WEB_SERVER_H