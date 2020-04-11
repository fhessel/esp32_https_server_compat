#ifndef ESPWEBSERVERSECURE_H
#define ESPWEBSERVERSECURE_H

#include "ESPWebServer.hpp"
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>

class ESPWebServerSecure;

// Placeholder class, to make the API conform to what Esp8266WebServerSecure provides.
class ESPWebServerUnderlyingServer {
  friend class ESPWebServerSecure;
protected:
  ESPWebServerUnderlyingServer(ESPWebServerSecure *webserver)
  : _webserver(webserver)
  {}
  ESPWebServerSecure* _webserver;
public:
  void setServerKeyAndCert(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen);
  void setServerKeyAndCert_P(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen);
};

class ESPWebServerSecure : public ESPWebServer {
  friend class ESPWebServerUnderlyingServer;
public:
  ESPWebServerSecure(IPAddress addr, int port = 442);
  ESPWebServerSecure(int port = 442);
  void setServerKeyAndCert(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen);
  void setServerKeyAndCert_P(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen);
  ESPWebServerUnderlyingServer& getServer() { return _underlyingServer; }
protected:
  ESPWebServerUnderlyingServer _underlyingServer;
  httpsserver::SSLCert _sslCert;
};

#endif //ESPWEBSERVERSECURE_H