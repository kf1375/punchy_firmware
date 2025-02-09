#include "dns_server.h"

#include "logging.h"

/**
 * @brief Construct a new DNSServer object
 *
 * @param mgr Mongoose event manager
 */
DnsServer::DnsServer(mg_mgr &mgr) : m_mgr(mgr) {}

/**
 * @brief Setup DNS server to reply for any domain requested with the local IP address
 */
void DnsServer::setup()
{
  String dnsUrl = "udp://0.0.0.0:53";
  LOG_INFO("Starting DNS server on" + dnsUrl);
  mg_listen(
      &m_mgr, dnsUrl.c_str(), [](mg_connection *c, int ev, void *ev_data) { static_cast<DnsServer *>(c->fn_data)->eventHandler(c, ev, ev_data); },
      this);
}

/**
 * @brief Dns server loop
 *
 */
void DnsServer::loop() {}

/**
 * @brief Handle incoming DNS requests, answer with the local IP address
 *
 * @param c Mongoose connection
 * @param ev Event
 * @param ev_data Event data
 */
void DnsServer::eventHandler(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_OPEN) {
    c->is_hexdumping = 0;
  } else if (ev == MG_EV_READ) {
    struct mg_dns_rr rr; // Parse first question, offset 12 is header size
    size_t n = mg_dns_parse_rr(c->recv.buf, c->recv.len, 12, true, &rr);
    if (n > 0) {
      char buf[512];
      struct mg_dns_header *h = (struct mg_dns_header *)buf;
      memset(buf, 0, sizeof(buf));                                    // Clear the whole datagram
      h->txnid = ((struct mg_dns_header *)c->recv.buf)->txnid;        // Copy tnxid
      h->num_questions = mg_htons(1);                                 // We use only the 1st question
      h->num_answers = mg_htons(1);                                   // And only one answer
      h->flags = mg_htons(0x8400);                                    // Authoritative response
      memcpy(buf + sizeof(*h), c->recv.buf + sizeof(*h), n);          // Copy question
      memcpy(buf + sizeof(*h) + n, m_dnsAnswer, sizeof(m_dnsAnswer)); // And answer
      mg_send(c, buf, 12 + n + sizeof(m_dnsAnswer));                  // And send it!
    }
    mg_iobuf_del(&c->recv, 0, c->recv.len);
    // c->is_draining = 1; //Do NOT drain DNS Server, because then it will shut down
    // and captive page will not work any more.
  }
}