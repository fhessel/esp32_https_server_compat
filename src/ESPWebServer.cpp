
#include "ESPWebServer.hpp"
#include <string>

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
  _server(HTTPServer(port, 4, addr)),
  _contentLength(0)
{
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
  _server.stop();
}

void ESPWebServer::stop() {
  _server.stop();
}

bool ESPWebServer::authenticate(const char * username, const char * password) {
  HTTPS_LOGE("authenticate() not yet implemented");
  return false;
}

void ESPWebServer::requestAuthentication(HTTPAuthMethod mode, const char* realm, const String& authFailMsg) {
  if (realm == NULL) realm = "Login Required";
  if (mode == BASIC_AUTH) {
    std::string authArg = "Basic realm=\"";
    authArg += realm;
    authArg += "\"";
    _activeResponse->setHeader("WWW-Authenticate", authArg);
  } else {
    HTTPS_LOGE("Only BASIC_AUTH implemented");
  }
}

void ESPWebServer::on(const String &uri, THandlerFunction handler) {
  on(uri, HTTP_GET, handler);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn) {
  on(uri, method, fn, _uploadHandler);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn) {
  // TODO: Handle HTTP_ANY
  const char *methodname = "???";
  for (size_t n = 0; n < sizeof(METHODNAMES); n++) {
    if (METHODNAMES[n].val == method) {
      methodname = METHODNAMES[n].text;
      break;
    }
  }
  ESPWebServerNode *node = new ESPWebServerNode(this,std::string(uri.c_str()), std::string(methodname),fn, ufn);
  _server.registerNode(node);
}

void ESPWebServer::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header) {
  ESPWebServerStaticNode *node = new ESPWebServerStaticNode(this, std::string(path), fs, path, cache_header);
  _server.registerNode(node);
}

void ESPWebServer::onNotFound(THandlerFunction fn) {
  if (_notFoundNode != nullptr) {
    delete _notFoundNode;
  }
  _notFoundNode = new ESPWebServerNode(this, "", "", fn, THandlerFunction());
  _server.setDefaultNode(_notFoundNode);
}

void ESPWebServer::onFileUpload(THandlerFunction fn) {
  _uploadHandler = fn;
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
  return *_activeUpload;
}

String ESPWebServer::pathArg(unsigned int i) {
  // TODO
  HTTPS_LOGE("pathArg() not yet implemented");
  return "";
}

String ESPWebServer::arg(String name) {
  ResourceParameters *params = _activeRequest->getParams();
  std::string value;
  params->getQueryParameter(std::string(name.c_str()), value);
  HTTPS_LOGD("arg(%s) returns %s", name.c_str(), value.c_str());
  return String(value.c_str());
}

String ESPWebServer::arg(int i) {
  // TODO
  HTTPS_LOGE("arg(int) not yet implemented");
  return "";
}

String ESPWebServer::argName(int i) {
  // TODO
  HTTPS_LOGE("argName() not yet implemented");
  return "";
}

int ESPWebServer::args() {
  // TODO
  HTTPS_LOGE("args() not yet implemented");
  return 0;
}

bool ESPWebServer::hasArg(String name) {
  ResourceParameters *params = _activeRequest->getParams();
  bool rv = params->isQueryParameterSet(std::string(name.c_str()));
  HTTPS_LOGD("hasArg(%s) returns %d", name.c_str(), (int)rv);
  return rv;
}

void ESPWebServer::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
  // TODO
  HTTPS_LOGE("collectHeaders() not yet implemented");
}

String ESPWebServer::header(String name) {
  // TODO
  HTTPS_LOGE("header(String) not yet implemented");
  return "";
}

String ESPWebServer::header(int i) {
  // TODO
  HTTPS_LOGE("header(int) not yet implemented");
  return "";
}

String ESPWebServer::headerName(int i) {
  // TODO
  HTTPS_LOGE("headerName() not yet implemented");
  return "";
}

int ESPWebServer::headers() {
  // TODO
  HTTPS_LOGE("headers() not yet implemented");
  return 0;
}

bool ESPWebServer::hasHeader(String name) {
  // TODO
  HTTPS_LOGE("hasHeader() not yet implemented");
  return false;
}

String ESPWebServer::hostHeader() {
  // TODO
  HTTPS_LOGE("hostHeader() not yet implemented");
  return "";
}

void ESPWebServer::send(int code, const char* content_type, const String& content) {
  _contentLength = content.length();
  _activeResponse->setStatusCode(code);
  _activeResponse->setHeader("Content-Type", content_type);
  _standardHeaders();
  _activeResponse->print(content);
}

void ESPWebServer::send(int code, char* content_type, const String& content) {
  send(code, (const char*)content_type, content);
}

void ESPWebServer::send(int code, const String& content_type, const String& content) {
  send(code, content_type.c_str(), content);
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content) {
  _contentLength = strlen_P(content);
  _activeResponse->setStatusCode(code);
  String memContentType(FPSTR(content_type));
  _activeResponse->setHeader("Content-Type", memContentType.c_str());
  _standardHeaders();
  _activeResponse->print(FPSTR(content));
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength) {
  _contentLength = contentLength;
  _activeResponse->setStatusCode(code);
  String memContentType(FPSTR(content_type));
  _activeResponse->setHeader("Content-Type", memContentType.c_str());
  _standardHeaders();
  _activeResponse->write((const uint8_t *)content, contentLength);
}

void ESPWebServer::_standardHeaders() {
  if (_contentLength != CONTENT_LENGTH_NOT_SET && _contentLength != CONTENT_LENGTH_UNKNOWN) {
    _activeResponse->setHeader("Content-Length", String(_contentLength).c_str());
  }
}

void ESPWebServer::enableCORS(boolean value) {
  if (value) _server.setDefaultHeader("Access-Control-Allow-Origin", "*");
}

void ESPWebServer::enableCrossOrigin(boolean value) {
  enableCORS(value);
}

void ESPWebServer::setContentLength(const size_t contentLength) {
  _contentLength = contentLength;
}

void ESPWebServer::sendHeader(const String& name, const String& value, bool first) {
  if (first) {
    HTTPS_LOGW("sendHeader(..., first=true) not implemented");
  }
  _activeResponse->setHeader(name.c_str(), value.c_str());
}

void ESPWebServer::sendContent(const String& content) {
  _activeResponse->print(content);
}

void ESPWebServer::sendContent_P(PGM_P content) {
  _activeResponse->print(FPSTR(content));
}

void ESPWebServer::sendContent_P(PGM_P content, size_t size) {
  _activeResponse->write((const uint8_t *)content, size);
}

String ESPWebServer::urlDecode(const String& text) {
  // TODO
  HTTPS_LOGE("urlDecode() not yet implemented");
  return text;
}

void ESPWebServer::_handlerWrapper(
  httpsserver::HTTPRequest *req,
  httpsserver::HTTPResponse *res) {
  ESPWebServerNode *node = (ESPWebServerNode*)req->getResolvedNode();
  node->_wrapper->_activeRequest = req;
  node->_wrapper->_activeResponse = res;
  node->_wrapper->_contentLength = CONTENT_LENGTH_NOT_SET;
  node->_wrappedHandler();
  node->_wrapper->_activeRequest = nullptr;
  node->_wrapper->_activeResponse = nullptr;
}

ESPWebServerNode::ESPWebServerNode(
  ESPWebServer *server,
  const std::string &path,
  const std::string &method,
  const THandlerFunction &handler,
  const THandlerFunction &uploadHandler,
  const std::string &tag) :
  ResourceNode(path, method, &(ESPWebServer::_handlerWrapper), tag),
  _wrapper(server),
  _wrappedHandler(handler),
  _wrappedUploadHandler(uploadHandler) {

}

ESPWebServerNode::~ESPWebServerNode() {

}

ESPWebServerStaticNode::ESPWebServerStaticNode(
  ESPWebServer *server,
  const std::string &path,
  FS& fs,
  const char *filePath,
  const char *cache_header)
: ResourceNode(path, "GET", &(ESPWebServerStaticNode::_handlerWrapper), ""),
  _fs(fs),
  _filePath(filePath),
  _cache_header(cache_header),
  _isFile(false)
{
  _isFile = _fs.exists(filePath);
}

void ESPWebServerStaticNode::_handler(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res) {
  HTTPS_LOGE("static not yet implemented path=%s filePath=%s\n", _path.c_str(), _filePath);
}

void ESPWebServerStaticNode::_handlerWrapper(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res) {
  ESPWebServerStaticNode *node = (ESPWebServerStaticNode*)req->getResolvedNode();
  node->_handler(req, res);
}


ESPWebServerStaticNode::~ESPWebServerStaticNode() {

}

