
#include "ESPWebServer.hpp"

using namespace httpsserver;

ESPWebServer::ESPWebServer(IPAddress addr, int port) {
  _server = new HTTPServer(port, 1, addr);
}

ESPWebServer::ESPWebServer(int port) {
  _server = new HTTPServer(port, 1);
}

ESPWebServer::~ESPWebServer() {
  delete _server;
}

void ESPWebServer::begin() {
  _server->start();
}

void ESPWebServer::handleClient() {
  _server->loop();
}

void ESPWebServer::close() {

}

void ESPWebServer::stop() {
  _server->stop();
}

bool ESPWebServer::authenticate(const char * username, const char * password) {
  return false;
}

void ESPWebServer::requestAuthentication(HTTPAuthMethod mode, const char* realm, const String& authFailMsg) {

}

void ESPWebServer::on(const String &uri, THandlerFunction handler) {
  
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn) {

}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn) {

}

void ESPWebServer::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header) {

}

void ESPWebServer::onNotFound(THandlerFunction fn) {

}

void ESPWebServer::onFileUpload(THandlerFunction fn) {

}

String ESPWebServer::uri() {

}

HTTPMethod ESPWebServer::method() {

}

HTTPUpload& ESPWebServer::upload() {
  HTTPUpload upload;

  return upload;
}

String ESPWebServer::pathArg(unsigned int i) {

}

String ESPWebServer::arg(String name) {

}

String ESPWebServer::arg(int i) {

}

String ESPWebServer::argName(int i) {

}

int ESPWebServer::args() {

}

bool ESPWebServer::hasArg(String name) {

}

void ESPWebServer::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {

}

String ESPWebServer::header(String name) {

}

String ESPWebServer::header(int i) {

}

String ESPWebServer::headerName(int i) {

}

int ESPWebServer::headers() {

}

bool ESPWebServer::hasHeader(String name) {

}

String ESPWebServer::hostHeader() {

}

void ESPWebServer::send(int code, const char* content_type, const String& content) {

}

void ESPWebServer::send(int code, char* content_type, const String& content) {

}

void ESPWebServer::send(int code, const String& content_type, const String& content) {

}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content) {

}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength) {

}

void ESPWebServer::enableCORS(boolean value) {

}

void ESPWebServer::enableCrossOrigin(boolean value) {

}

void ESPWebServer::setContentLength(const size_t contentLength) {

}

void ESPWebServer::sendHeader(const String& name, const String& value, bool first) {

}

void ESPWebServer::sendContent(const String& content) {

}

void ESPWebServer::sendContent_P(PGM_P content) {

}

void ESPWebServer::sendContent_P(PGM_P content, size_t size) {

}

String ESPWebServer::urlDecode(const String& text) {

}
