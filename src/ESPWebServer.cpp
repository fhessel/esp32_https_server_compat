
#include "ESPWebServer.hpp"
#include <HTTPMultipartBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>
#include <string>
#include <map>
#include <libb64/cencode.h>

using namespace httpsserver;

/* Copy the content of Arduino String s into a newly allocated char array p */
#define ARDUINOTONEWCHARARR(s,p) {size_t sLen=s.length()+1;char *c=new char[sLen];c[sLen-1]=0;s.toCharArray(p,sLen);p=c;}

class BodyResourceParameters : public ResourceParameters {
  friend class ESPWebServer;
};

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

// We need to specify some content-type mapping, so the resources get delivered with the
// right content type and are displayed correctly in the browser
char contentTypes[][2][32] = {
  {".html", "text/html"},
  {".txt", "text/plain"},
  {".css",  "text/css"},
  {".js",   "application/javascript"},
  {".json", "application/json"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};

ESPWebServer::ESPWebServer(HTTPServer *server) :
  _server(server),
  _contentLength(0)
{
  _notFoundNode = nullptr;
}

ESPWebServer::ESPWebServer(IPAddress addr, int port) :
  _server(new HTTPServer(port, 4, addr)),
  _contentLength(0)
{
  _notFoundNode = nullptr;
}

ESPWebServer::ESPWebServer(int port) :
  _server(new HTTPServer(port, 4)) {
  _notFoundNode = nullptr;
}

ESPWebServer::~ESPWebServer() {
  if (_notFoundNode != nullptr) {
    delete _notFoundNode;
  }
}

void ESPWebServer::begin() {
  _server->start();
}

void ESPWebServer::handleClient() {
  _server->loop();
}

void ESPWebServer::close() {
  _server->stop();
}

void ESPWebServer::stop() {
  _server->stop();
}

bool ESPWebServer::authenticate(const char * username, const char * password) {
  std::string authHeader = _activeRequest->getHeader("Authorization");
  if (authHeader == "") return false;
  if (authHeader.substr(0, 5) == "Basic") {
    std::string reqUser = _activeRequest->getBasicAuthUser();
    std::string reqPassword = _activeRequest->getBasicAuthPassword();
    return (username == reqUser && password == reqPassword);
  } else if (authHeader.substr(0, 6) == "Digest") {
    HTTPS_LOGE("Only BASIC_AUTH implemented");
#if 0
    std::string authReq = authHeader.substr(6);
    std::string realm = _extractParam(authReq, "realm=\"");
    std::string hash = credentialHash(username, _realm, password);
    return authenticateDigest(username, hash);
#endif
  }
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
#if 0
    _snonce = _getRandomHexString();
    _sopaque = _getRandomHexString();
    std::string authArg = "Digest realm=\"";
    authArg += realm;
    authArg += "\", qop=\"auth\", nonce=\"";
    authArg += _snonce;
    authArg += "\", opaque=\"";
    authArg += _sopaque;
    authArg += "\"";
    _activeResponse->setHeader("WWW-Authenticate", authArg);
#endif
  }
  send(401, "text/html", authFailMsg);
}

void ESPWebServer::on(const String &uri, THandlerFunction handler) {
  on(uri, HTTP_GET, handler);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn) {
  on(uri, method, fn, _uploadHandler);
}

void ESPWebServer::on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn) {
  // TODO: Handle HTTP_ANY
  // TODO: convert {} in uri to * (so pathArg() will work)
  const char *methodname = "???";
  for (size_t n = 0; n < sizeof(METHODNAMES); n++) {
    if (METHODNAMES[n].val == method) {
      methodname = METHODNAMES[n].text;
      break;
    }
  }
  ESPWebServerNode *node = new ESPWebServerNode(this,std::string(uri.c_str()), std::string(methodname),fn, ufn);
  _server->registerNode(node);
}

void ESPWebServer::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header) {
  std::string wsUri(uri);
  std::string wsPath(path);
  if (wsUri[wsUri.length()-1] == '/') {
    // serving a whole directory
    wsUri += "*";
    if (wsPath[wsPath.length()-1] != '/') wsPath += "/";
  }
  ESPWebServerStaticNode *node = new ESPWebServerStaticNode(this, wsUri, fs, wsPath, std::string(cache_header?cache_header:""));
  _server->registerNode(node);
}

void ESPWebServer::onNotFound(THandlerFunction fn) {
  if (_notFoundNode != nullptr) {
    delete _notFoundNode;
  }
  _notFoundNode = new ESPWebServerNode(this, "", "", fn, THandlerFunction());
  _server->setDefaultNode(_notFoundNode);
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
  auto rv = _activeParams->getPathParameter(i);
  return String(rv.c_str());
}

String ESPWebServer::arg(String name) {
  // Special case: arg("plain") returns the body of non-multipart requests.
  if (name == "plain") {
    bool isForm = false;
    HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
    HTTPHeader* ctHeader = headers->get("Content-Type");
    if (ctHeader && ctHeader->_value.substr(0, 10) == "multipart/") {
      isForm = true;
    }
    if (!isForm) {
      size_t bodyLength = _activeRequest->getContentLength();
      String rv;
      rv.reserve(bodyLength);
      char buffer[257];
      while(!_activeRequest->requestComplete()) {
        size_t readLength = _activeRequest->readBytes((byte*)buffer, 256);
        if (readLength <= 0) break;
        buffer[readLength] = 0;
        rv += buffer;
      }
      return rv;
    }
  }
  std::string value;
  _activeParams->getQueryParameter(std::string(name.c_str()), value);
  return String(value.c_str());
}

String ESPWebServer::arg(int i) {
  int idx=0;
  for (auto it=_activeParams->beginQueryParameters(); it != _activeParams->endQueryParameters(); it++, idx++) {
    if (idx == i)
      return String(it->second.c_str());
  }
  return "";
}

String ESPWebServer::argName(int i) {
  int idx=0;
  for (auto it=_activeParams->beginQueryParameters(); it != _activeParams->endQueryParameters(); it++, idx++) {
    if (idx == i)
      return String(it->first.c_str());
  }
  return "";
}

int ESPWebServer::args() {
  return _activeParams->getQueryParameterCount();
}

bool ESPWebServer::hasArg(String name) {
  bool rv = _activeParams->isQueryParameterSet(std::string(name.c_str()));
  return rv;
}

void ESPWebServer::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
  HTTPS_LOGE("collectHeaders() not implemented");
}

String ESPWebServer::header(String name) {
  HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
  HTTPHeader* header = headers->get(std::string(name.c_str()));
  if (header) {
    return String(header->_value.c_str());
  }
  return "";
}

String ESPWebServer::header(int i) {
  HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
  auto allHeaders = headers->getAll();
  if (i >= 0 && i < allHeaders->size()) {
    HTTPHeader* header = allHeaders->at(i);
    return String(header->_value.c_str());
  }
  return "";
}

String ESPWebServer::headerName(int i) {
  HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
  auto allHeaders = headers->getAll();
  if (i >= 0 && i < allHeaders->size()) {
    HTTPHeader* header = allHeaders->at(i);
    return String(header->_name.c_str());
  }
  return "";
}

int ESPWebServer::headers() {
  HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
  auto allHeaders = headers->getAll();
  return allHeaders->size();
}

bool ESPWebServer::hasHeader(String name) {
  HTTPHeaders* headers = _activeRequest->getHTTPHeaders();
  HTTPHeader* header = headers->get(std::string(name.c_str()));
  return header != NULL;
}

String ESPWebServer::hostHeader() {
  return header("Host");
}

void ESPWebServer::_prepareStreamFile(size_t fileSize, const String& contentType) {
  _contentLength = fileSize;
  _activeResponse->setStatusCode(200);
  _activeResponse->setHeader("Content-Type", contentType.c_str());
  _standardHeaders();
}

void ESPWebServer::send(int code, const char* content_type, const String& content) {
  if (_contentLength == CONTENT_LENGTH_NOT_SET) _contentLength = content.length();
  _activeResponse->setStatusCode(code);
  if (content_type != NULL) {
    _activeResponse->setHeader("Content-Type", content_type);
  }
  _standardHeaders();
  if (content) {
    _activeResponse->print(content);
  }
}

void ESPWebServer::send(int code, char* content_type, const String& content) {
  send(code, (const char*)content_type, content);
}

void ESPWebServer::send(int code, const String& content_type, const String& content) {
  send(code, content_type.c_str(), content);
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content) {
  if (_contentLength == CONTENT_LENGTH_NOT_SET) _contentLength = strlen_P(content);
  _activeResponse->setStatusCode(code);
  String memContentType(FPSTR(content_type));
  _activeResponse->setHeader("Content-Type", memContentType.c_str());
  _standardHeaders();
  _activeResponse->print(FPSTR(content));
}

void ESPWebServer::send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength) {
  if (_contentLength == CONTENT_LENGTH_NOT_SET) _contentLength = contentLength;
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
  if (value) _server->setDefaultHeader("Access-Control-Allow-Origin", "*");
}

void ESPWebServer::enableCrossOrigin(boolean value) {
  enableCORS(value);
}

void ESPWebServer::setContentLength(const size_t contentLength) {
  _contentLength = contentLength;
}

void ESPWebServer::sendHeader(const String& name, const String& value, bool first) {
  if (first) {
    HTTPS_LOGE("sendHeader(..., first=true) not implemented");
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
  auto decoded = ::urlDecode(std::string(text.c_str()));
  return String(decoded.c_str());
}

void ESPWebServer::_handlerWrapper(
  httpsserver::HTTPRequest *req,
  httpsserver::HTTPResponse *res) {
  BodyResourceParameters *bodyParams = nullptr;
  ESPWebServerNode *node = (ESPWebServerNode*)req->getResolvedNode();
  node->_wrapper->_activeRequest = req;
  node->_wrapper->_activeResponse = res;
  node->_wrapper->_activeParams = req->getParams();
  node->_wrapper->_contentLength = CONTENT_LENGTH_NOT_SET;
  // POST form data needs to be handled specially
  if (req->getMethod() == "POST") {
    HTTPBodyParser *parser = NULL;
    std::string contentType = req->getHeader("Content-Type");
    size_t semicolonPos = contentType.find(";");
    if (semicolonPos != std::string::npos) {
      contentType = contentType.substr(0, semicolonPos);
    }
    if (contentType == "multipart/form-data") {
      parser = new HTTPMultipartBodyParser(req); 
    }
    if (contentType == "application/x-www-form-urlencoded") {
      parser = new HTTPURLEncodedBodyParser(req);
    }
    //
    // _activeParams should be the merger of the URL parameters and the body parameters.
    //
    bodyParams = new BodyResourceParameters();
    for (auto it = node->_wrapper->_activeParams->beginQueryParameters(); it != node->_wrapper->_activeParams->endQueryParameters(); it++) {
      bodyParams->setQueryParameter(it->first, it->second);
    }
    node->_wrapper->_activeParams = bodyParams;

    while (parser && parser->nextField()) {
      std::string name = parser->getFieldName();
      std::string filename = parser->getFieldFilename();
      if (filename != "") {
        // This field is a file. Use the uploader
        std::string mimeType = parser->getFieldMimeType();
        HTTPUpload uploader;
        node->_wrapper->_activeUpload = &uploader;
        uploader.status = UPLOAD_FILE_START;
        uploader.name = String(name.c_str());
        uploader.filename = String(filename.c_str());
        uploader.type = String(mimeType.c_str());
        uploader.totalSize = 0;
        uploader.currentSize = 0;
        // First call to the uploader callback
        node->_wrappedUploadHandler();
        // Now loop over the data
        uploader.status = UPLOAD_FILE_WRITE;
        while(!parser->endOfField()) {
          uploader.currentSize = parser->read(uploader.buf, sizeof(uploader.buf));
          uploader.totalSize += uploader.currentSize;
          node->_wrappedUploadHandler();
        }
        uploader.status = UPLOAD_FILE_END;
        node->_wrappedUploadHandler();
        node->_wrapper->_activeUpload = NULL;
      } else {
        // This field is not a file. Add the value
        std::string value("");
        while (!parser->endOfField()) {
          byte buf[512];
          size_t readLength = parser->read(buf, 512);
          std::string bufString((char *)buf, readLength);
          value += bufString;
        }
        bodyParams->setQueryParameter(name, value);
      }
    }
    delete parser;
  }
  node->_wrappedHandler();
  node->_wrapper->_activeRequest = nullptr;
  node->_wrapper->_activeResponse = nullptr;
  node->_wrapper->_activeParams = nullptr;
}

/**
 * This handler function will try to load the requested resource from SPIFFS's /public folder.
 * 
 * If the method is not GET, it will throw 405, if the file is not found, it will throw 404.
 */
void ESPWebServer::_staticPageHandler(HTTPRequest * req, HTTPResponse * res) {
  assert(req->getMethod() == "GET");
  ESPWebServerStaticNode *node = (ESPWebServerStaticNode*)req->getResolvedNode();
  // xxxjack remove urlpath bits from reqFile
  // xxxjack add index.htm if needed
  // xxxjack prepend filepath
  // Redirect / to /index.html
  std::string reqFile;
  if (!req->getParams()->getPathParameter(0, reqFile)) reqFile = "index.html";

  // Try to open the file
  std::string filename = node->_filePath + reqFile;

  // Check if the file exists
  if (!node->_fileSystem.exists(filename.c_str())) {
    // Send "404 Not Found" as response, as the file doesn't seem to exist
    res->setStatusCode(404);
    res->setStatusText("Not found");
    res->println("404 Not Found");
    return;
  }

  File file = node->_fileSystem.open(filename.c_str());

  // Set length
  res->setHeader("Content-Length", httpsserver::intToString(file.size()));

  // Content-Type is guessed using the definition of the contentTypes-table defined above
  int cTypeIdx = 0;
  do {
    if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
      res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
      break;
    }
    cTypeIdx+=1;
  } while(strlen(contentTypes[cTypeIdx][0])>0);

  // Read the file and write it to the response
  uint8_t buffer[256];
  size_t length = 0;
  do {
    length = file.read(buffer, 256);
    res->write(buffer, length);
  } while (length > 0);

  file.close();
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
  const std::string& urlPath,
  FS& fs,
  const std::string& filePath,
  const std::string& cache_header) :
  ResourceNode(urlPath, "GET", &(ESPWebServer::_staticPageHandler), ""),
  _filePath(filePath),
  _fileSystem(fs),
  _cache_header(cache_header)
{
}

ESPWebServerStaticNode::~ESPWebServerStaticNode() {
}
