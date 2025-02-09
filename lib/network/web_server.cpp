#include "web_server.h"

#include "logging.h"
#include "util.h"
#include <WiFi.h>

/**
 * @brief Construct a new WebServer object
 *
 */
WebServer::WebServer(mg_mgr &mgr, Configuration &config, HardwareController &hwController)
    : m_mgr(mgr), m_config(config), m_hwController(hwController)
{
}

/**
 * @brief Webserver Setup to handle all incoming http requests
 *
 */
void WebServer::setup()
{
  String httpUrl = "http://0.0.0.0:80";
  LOG_INFO("Starting HTTP listener on " + httpUrl);
  m_conn = mg_http_listen(
      &m_mgr, httpUrl.c_str(), [](mg_connection *c, int ev, void *ev_data) { static_cast<WebServer *>(c->fn_data)->eventHandler(c, ev, ev_data); },
      this);
}

/**
 * @brief
 *
 */
void WebServer::stop()
{
  if (m_conn != nullptr) {
    LOG_INFO("Stopping HTTP listener");
    mg_close_conn(m_conn);
    (*this).~WebServer();
    WiFi.softAPdisconnect(true);
  }
}

/**
 * @brief Webserver loop
 *
 */
void WebServer::loop() {}

/**
 * @brief Mongoose event handler for http requests
 *
 * match API requests to the corresponding functions and send filesystem static files
 * if no api url matches
 */
void WebServer::eventHandler(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    String url = String(hm->uri.buf, hm->uri.len);
    LOG_INFO(url);
    if (mg_match(hm->uri, mg_str("/api/wifi/set"), NULL)) {
      handleSetWifi(c, hm);
    } else if (mg_match(hm->uri, mg_str("/api/wifi/get"), NULL)) {
      handleGetWifi(c, hm);
    } else if (mg_match(hm->uri, mg_str("/hotspot-detect.html"), NULL)) {
      mg_http_serve_file(c, hm, "/littlefs/static/captive_page.html", &m_httpServeOpts);
    } else if (mg_match(hm->uri, mg_str("/generate_204"), NULL)) {
      mg_http_serve_file(c, hm, "/littlefs/static/captive_page.html", &m_httpServeOpts);
    } else if (mg_match(hm->uri, mg_str("/gen_204"), NULL)) {
      mg_http_serve_file(c, hm, "/littlefs/static/captive_page.html", &m_httpServeOpts);
    } else if (mg_match(hm->uri, mg_str("/canonical.html"), NULL)) {
      mg_http_serve_file(c, hm, "/littlefs/static/captive_page.html", &m_httpServeOpts);
    } else if (mg_match(hm->uri, mg_str("/"), NULL)) {
      mg_http_serve_file(c, hm, "/littlefs/static/captive_page.html", &m_httpServeOpts);
    } else {
      // by default serve static files
      mg_http_serve_dir(c, hm, &m_httpServeOpts);
    }
    // Delete received data
    c->recv.len = 0;
    c->is_draining = 1;
  } else if (ev == MG_EV_ACCEPT) {
    if (Util::getNumberOfConnections(c->mgr) > 20) {
      MG_ERROR(("Too many connections"));
      c->is_closing = 1;
    }
  } else if (ev == MG_EV_ERROR) {
    // on error flush the mg_io_buff
    c->is_closing = 1;
  }
}

void WebServer::handleSetWifi(struct mg_connection *c, struct mg_http_message *hm)
{
  String json_str = String(hm->body.buf, hm->body.len);

  JsonDocument credentials;
  deserializeJson(credentials, json_str);

  String nssid = credentials["ssid"].as<String>();
  String npassword = credentials["password"].as<String>();

  LOG_INFO(nssid);
  LOG_INFO(npassword);

  // Save the new WiFi credentials to the wifi.txt file
  m_config.wifi.setSSID(nssid);
  m_config.wifi.setPassword(npassword);

  JsonDocument doc;
  doc["status"] = "OK";
  sendJsonResponse(c, doc);
}

void WebServer::handleGetWifi(struct mg_connection *c, struct mg_http_message *hm)
{
  JsonDocument doc;
  typedef enum { CONNECTED = 0, SSID_NOT_AVAILABLE, CONNECTION_FAILED, DISCONNECTED } connection_status_t;
  connection_status_t connection_status = DISCONNECTED;

  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    connection_status = CONNECTED;
    doc["mac"] = WiFi.macAddress();
    mg_timer_add(&m_mgr, 2000, 0, [](void *arg) { static_cast<WebServer *>(arg)->stop(); }, this); // stop the server after 5 seconds

  } else if (m_config.wifi.failed()) {
    if (status == WL_NO_SSID_AVAIL) {
      connection_status = SSID_NOT_AVAILABLE;
    } else {
      connection_status = CONNECTION_FAILED;
    }
  }

  doc["status"] = connection_status;
  LOG_INFO("Connection status: " + String(connection_status));
  sendJsonResponse(c, doc);
}

void WebServer::sendJsonResponse(struct mg_connection *c, JsonDocument &output)
{
  String json;
  serializeJson(output, json);
  mg_http_reply(c, 200, "Content-Type: application/json\r\nConnection: close\r\n", "%s\n", json.c_str());
}

void WebServer::sendJsonResponse(struct mg_connection *c, JsonObject &output)
{
  String json;
  serializeJson(output, json);
  mg_http_reply(c, 200, "Content-Type: application/json\r\nConnection: close\r\n", "%s\n", json.c_str());
}

void WebServer::sendJsonResponse(struct mg_connection *c, JsonArray &output)
{
  String json;
  serializeJson(output, json);
  mg_http_reply(c, 200, "Content-Type: application/json\r\nConnection: close\r\n", "%s\n", json.c_str());
}

void WebServer::sendTextResponse(struct mg_connection *c, size_t code, String text)
{
  mg_http_reply(c, code, "Content-Type: text/plain\r\nConnection: close\r\n", "%s", text.c_str());
}

void WebServer::sendTextResponse(struct mg_connection *c, size_t code, const char *text)
{
  mg_http_reply(c, code, "Content-Type: text/plain\r\nConnection: close\r\n", "%s", text);
}

String WebServer::extractStringVariable(struct mg_http_message *hm, String variable)
{
  struct mg_http_part part;
  size_t ofs = 0;
  while ((ofs = mg_http_next_multipart(hm->body, ofs, &part)) > 0) {
    if (variable == String(part.name.buf, part.name.len)) {
      return String((char *)part.body.buf, part.body.len);
    }
  }
  return "";
}

bool WebServer::extractBoolVariable(struct mg_http_message *hm, String variable)
{
  return extractStringVariable(hm, variable) == "true";
}

float WebServer::extractFloatVariable(struct mg_http_message *hm, String variable)
{
  return strtof(extractStringVariable(hm, variable).c_str(), NULL);
}
int WebServer::extractIntVariable(struct mg_http_message *hm, String variable)
{
  return extractStringVariable(hm, variable).toInt();
}