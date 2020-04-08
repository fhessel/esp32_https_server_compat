#include "ESPWebServerSecure.hpp"


ESPWebServerSecure::ESPWebServerSecure(IPAddress addr, int port)
: ESPWebServer(new httpsserver::HTTPSServer(&_sslCert, port, 4, addr)),
  _underlyingServer(this),
  _sslCert(),
  keyStore(NULL),
  certStore(NULL)
{}

ESPWebServerSecure::ESPWebServerSecure(int port)
: ESPWebServer(new httpsserver::HTTPSServer(&_sslCert, port, 4)),
  _underlyingServer(this),
  _sslCert(),
  keyStore(NULL),
  certStore(NULL)
{}

void ESPWebServerSecure::setServerKeyAndCert(const uint8_t *key, int keyLen, const uint8_t *cert, int certLen) {
  if (keyStore) free(keyStore);
  if (certStore) free(certStore);
  keyStore = (uint8_t *)malloc(keyLen);
  certStore = (uint8_t *)malloc(certLen);
  if (keyStore == NULL || certStore == NULL) {
    HTTPS_LOGE("Out of memory for key and cert");
    return;
  }
  memcpy_P(keyStore, key, keyLen);
  memcpy_P(certStore, cert, certLen);
  _sslCert.setPK(keyStore, keyLen);
  _sslCert.setCert(certStore, certLen);
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
