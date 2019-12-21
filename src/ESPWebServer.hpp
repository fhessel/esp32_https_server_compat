#ifndef ESPWEBSERVER_H
#define ESPWEBSERVER_H

#include <functional>
#include <string>

#include <Arduino.h>
#include <FS.h>

#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPSCallbackFunction.hpp>

#include "HTTP_Method.h"

enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END,
                        UPLOAD_FILE_ABORTED };
enum HTTPClientStatus { HC_NONE, HC_WAIT_READ, HC_WAIT_CLOSE };
enum HTTPAuthMethod { BASIC_AUTH, /* DIGEST_AUTH */ };

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 1436
#endif

#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)

typedef std::function<void(void)> THandlerFunction;

typedef struct {
  HTTPUploadStatus status;
  String  filename;
  String  name;
  String  type;
  size_t  totalSize;    // file size
  size_t  currentSize;  // size of data currently in buf
  uint8_t buf[HTTP_UPLOAD_BUFLEN];
} HTTPUpload;

namespace fs {
class FS;
}

class ESPWebServerNode;

class ESPWebServer
{
public:
  ESPWebServer(IPAddress addr, int port = 80);
  ESPWebServer(int port = 80);
  virtual ~ESPWebServer();

  virtual void begin();
  virtual void handleClient();

  virtual void close();
  void stop();

  bool authenticate(const char * username, const char * password);
  void requestAuthentication(HTTPAuthMethod mode = BASIC_AUTH, const char* realm = NULL, const String& authFailMsg = String("") );

  void on(const String &uri, THandlerFunction handler);
  void on(const String &uri, HTTPMethod method, THandlerFunction fn);
  void on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn);
  void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header = NULL );
  void onNotFound(THandlerFunction fn);  //called when handler is not assigned
  void onFileUpload(THandlerFunction fn); //handle file uploads

  String uri();
  HTTPMethod method();
  HTTPUpload& upload();

  String pathArg(unsigned int i); // get request path argument by number
  String arg(String name);        // get request argument value by name
  String arg(int i);              // get request argument value by number
  String argName(int i);          // get request argument name by number
  int args();                     // get arguments count
  bool hasArg(String name);       // check if argument exists
  void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
  String header(String name);      // get request header value by name
  String header(int i);              // get request header value by number
  String headerName(int i);          // get request header name by number
  int headers();                     // get header count
  bool hasHeader(String name);       // check if header exists

  String hostHeader();            // get request host header if available or empty String if not

  // send response to the client
  // code - HTTP response code, can be 200 or 404
  // content_type - HTTP content type, like "text/plain" or "image/png"
  // content - actual content body
  void send(int code, const char* content_type = NULL, const String& content = String(""));
  void send(int code, char* content_type, const String& content);
  void send(int code, const String& content_type, const String& content);
  void send_P(int code, PGM_P content_type, PGM_P content);
  void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength);

  void enableCORS(boolean value = true);
  void enableCrossOrigin(boolean value = true);

  void setContentLength(const size_t contentLength);
  void sendHeader(const String& name, const String& value, bool first = false);
  void sendContent(const String& content);
  void sendContent_P(PGM_P content);
  void sendContent_P(PGM_P content, size_t size);

  static String urlDecode(const String& text);

  //template<typename T> size_t streamFile(T &file, const String& contentType);

protected:
  friend class ESPWebServerNode;

  /** The wrapper function that maps on() calls */
  static void _handlerWrapper(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);

  /** Add standard headers */
  void _standardHeaders();

  /** The backing server instance */
  httpsserver::HTTPServer _server;

  /** The currently active request */
  httpsserver::HTTPRequest *_activeRequest;
  httpsserver::HTTPResponse *_activeResponse;

  /** default node */
  ESPWebServerNode *_notFoundNode;

  /** Instance variables for standard headers */
  size_t _contentLength;
};

class ESPWebServerNode : public httpsserver::ResourceNode {
public:
  ESPWebServerNode(
    ESPWebServer *server,
    const std::string &path,
    const std::string &method,
    const THandlerFunction &handler,
    const std::string &tag = "");
  virtual ~ESPWebServerNode();

protected:
  friend class ESPWebServer;
  ESPWebServer *_wrapper;
  const THandlerFunction _wrappedHandler;
};

class ESPWebServerStaticNode : public httpsserver::ResourceNode {
public:
  ESPWebServerStaticNode(
    ESPWebServer *server,
    const std::string &path,
    FS& fs,
    const char *filePath,
    const char *cache_header);
  virtual ~ESPWebServerStaticNode();

protected:
  static void _handlerWrapper(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);
  void _handler(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);
  friend class ESPWebServer;
  ESPWebServer *_wrapper;
  FS& _fs;
  const char *_filePath;
  const char *_cache_header;
  bool _isFile;
};

#endif //ESPWEBSERVER_H