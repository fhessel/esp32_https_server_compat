#include "ESPWebServerSecure.hpp"


ESPWebServerSecure::ESPWebServerSecure(IPAddress addr, int port)
: ESPWebServer(new httpsserver::HTTPSServer(&_sslCert, port, 4, addr)),
  _underlyingServer(this),
  _sslCert()
{}

ESPWebServerSecure::ESPWebServerSecure(int port)
: ESPWebServer(new httpsserver::HTTPSServer(&_sslCert, port, 4)),
  _underlyingServer(this),
  _sslCert()
{}

void ESPWebServerSecure::setServerKeyAndCert(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen) {
  _sslCert.setPK((unsigned char *)key, keyLen);
  _sslCert.setCert((unsigned char *)cert, certLen);
}

void ESPWebServerSecure::setServerKeyAndCert_P(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen) {
  setServerKeyAndCert(key, keyLen, cert, certLen);
}

void ESPWebServerUnderlyingServer::setServerKeyAndCert(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen) {
  _webserver->setServerKeyAndCert(key, keyLen, cert, certLen);
}
void ESPWebServerUnderlyingServer::setServerKeyAndCert_P(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen) {
  _webserver->setServerKeyAndCert_P(key, keyLen, cert, certLen);
}
