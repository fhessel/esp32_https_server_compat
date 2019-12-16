
#include "ESPWebServer.hpp"

using namespace httpsserver;

/* Copy the content of Arduino String s into a newly allocated char array p */
#define ARDUINOTONEWCHARARR(s,p) {size_t sLen=s.length()+1;char *c=new char[sLen];c[sLen-1]=0;s.toCharArray(p,sLen);p=c;}

struct {
  int val;
  char text[8];
} METHODNAMES[] = {
  {HTTP_GET, "GET"},
  {HTTP_POST, "POST"},
  {HTTP_DELETE, "DELETE"},
  {HTTP_PUT, "PUT"},
  {HTTP_PATCH, "PATCH"},
  {HTTP_HEAD, "HEAD"},
  {HTTP_OPTIONS, "OPTIONS"},
};

ESPWebServer::ESPWebServer(IPAddress addr, int port) :
  _server(HTTPServer(port, 4, addr)) {
  _notFoundNode = nullptr;
}

ESPWebServer::ESPWebServer(int port) :
  _server(HTTPServer(port, 4)) {
  _notFoundNode = nullptr;
}

ESPWebServer::~ESPWebServer() {
  if (_notFoundNode != nullptr) {
    delete _notFoundNode;
  }
}

void ESPWebServer::begin() {
  _server.start();
}

void ESPWebServer::handleClient() {
  _server.loop();
}

void ESPWebServer::close() {
  // TODO
}

void ESPWebServer::stop() {
  _server.stop();
}

bool ESPWebServer::authenticate(const char * username, const char * password) {
  return false;
}

void ESPWebServer::requestAuthentication(HTTPAuthMethod mode, const char* realm, const String& authFailMsg) {
  // TODO
}

void ESPWebServer::on(const String &uri, THandlerFunction handler) {
  on(uri, HTTP_GET, handler);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn) {
  // TODO: Handle HTTP_ANY
  char *methodname = "???";
  for (size_t n = 0; n < sizeof(METHODNAMES); n++) {
    if (METHODNAMES[n].val == method) {
      methodname = METHODNAMES[n].text;
      break;
    }
  }
  ESPWebServerNode *node = new ESPWebServerNode(this,std::string(uri.c_str()), std::string(methodname),fn);
  _server.registerNode(node);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn) {
  // TODO
  // ufn handles uploads
}

void ESPWebServer::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header) {
  // TODO
}

void ESPWebServer::onNotFound(THandlerFunction fn) {
  if (_notFoundNode != nullptr) {
    delete _notFoundNode;
  }
  _notFoundNode = new ESPWebServerNode(this, "", "", fn);
  _server.setDefaultNode(_notFoundNode);
}

void ESPWebServer::onFileUpload(THandlerFunction fn) {
  // TODO
}

String ESPWebServer::uri() {
  return String(_activeRequest->getRequestString().c_str());
}

HTTPMethod ESPWebServer::method() {
  for(size_t n = 0; n < sizeof(METHODNAMES); n++) {
    if (!_activeRequest->getMethod().compare(METHODNAMES[n].text)) {
      return (HTTPMethod)METHODNAMES[n].val;
    }
  }
  return HTTP_ANY;
}

HTTPUpload& ESPWebServer::upload() {
  // TODO
  HTTPUpload upload;
  return upload;
}

String ESPWebServer::pathArg(unsigned int i) {
  // TODO
  return "";
}

String ESPWebServer::arg(String name) {
  // TODO
  return "";
}

String ESPWebServer::arg(int i) {
  // TODO
  return "";
}

String ESPWebServer::argName(int i) {
  // TODO
  return "";
}

int ESPWebServer::args() {
  // TODO
  return 0;
}

bool ESPWebServer::hasArg(String name) {
  // TODO
  return false;
}

void ESPWebServer::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
  // TODO
}

String ESPWebServer::header(String name) {
  // TODO
  return "";
}

String ESPWebServer::header(int i) {
  // TODO
  return "";
}

String ESPWebServer::headerName(int i) {
  // TODO
  return "";
}

int ESPWebServer::headers() {
  // TODO
  return 0;
}

bool ESPWebServer::hasHeader(String name) {
  // TODO
  return false;
}

String ESPWebServer::hostHeader() {
  // TODO
  return "";
}

void ESPWebServer::send(int code, const char* content_type, const String& content) {
  _activeResponse->setStatusCode(code);
  _activeResponse->setHeader("Content-Type", content_type);
  _activeResponse->print(content);
}

void ESPWebServer::send(int code, char* content_type, const String& content) {
  send(code, (const char*)content_type, content);
}

void ESPWebServer::send(int code, const String& content_type, const String& content) {
  send(code, content_type.c_str(), content);
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content) {
  // TODO
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength) {
  // TODO
}

void ESPWebServer::enableCORS(boolean value) {
  // TODO
}

void ESPWebServer::enableCrossOrigin(boolean value) {
  // TODO
}

void ESPWebServer::setContentLength(const size_t contentLength) {
  // TODO
}

void ESPWebServer::sendHeader(const String& name, const String& value, bool first) {
  // TODO
}

void ESPWebServer::sendContent(const String& content) {
  // TODO
}

void ESPWebServer::sendContent_P(PGM_P content) {
  // TODO
}

void ESPWebServer::sendContent_P(PGM_P content, size_t size) {
  // TODO
}

String ESPWebServer::urlDecode(const String& text) {
  // TODO
  return text;
}

void ESPWebServer::_handlerWrapper(
  httpsserver::HTTPRequest *req,
  httpsserver::HTTPResponse *res) {
  ESPWebServerNode *node = (ESPWebServerNode*)req->getResolvedNode();
  node->_wrapper->_activeRequest = req;
  node->_wrapper->_activeResponse = res;
  node->_wrappedHandler();
  node->_wrapper->_activeRequest = nullptr;
  node->_wrapper->_activeResponse = nullptr;
}

ESPWebServerNode::ESPWebServerNode(
  ESPWebServer *server,
  const std::string &path,
  const std::string &method,
  const THandlerFunction &handler,
  const std::string &tag) :
  ResourceNode(path, method, &(ESPWebServer::_handlerWrapper), tag),
  _wrapper(server),
  _wrappedHandler(handler) {

}

ESPWebServerNode::~ESPWebServerNode() {

}
