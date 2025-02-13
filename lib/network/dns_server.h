#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <Arduino.h>
#include <mongoose.h>

class DnsServer
{
public:
  DnsServer(mg_mgr &mgr);

  void setup();
  void loop();

private:
  struct mg_mgr &m_mgr;
  const uint8_t m_dnsAnswer[16] = {
      0xc0, 0x0c,          // Point to the name in the DNS question
      0,    1,             // 2 bytes - record type, A
      0,    1,             // 2 bytes - address class, INET
      0,    0,    0,  120, // 4 bytes - TTL
      0,    4,             // 2 bytes - address length
      172,  217,  28, 1    // 4 bytes - IP address
  };

  void eventHandler(struct mg_connection *c, int ev, void *ev_data);
};
#endif // DNS_SERVER_H