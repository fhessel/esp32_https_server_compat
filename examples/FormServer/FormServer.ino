#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#ifdef USE_DEFAULT_WEBSERVER
#include <WebServer.h>
typedef WebServer ESPWebServer;
#else
#include <ESPWebServer.hpp>
#endif

const char* ssid = "........";
const char* password = "........";

ESPWebServer server(80);

static std::string htmlEncode(std::string data) {
  const char *p = data.c_str();
  std::string rv = "";
  while(p && *p) {
    char escapeChar = *p++;
    switch(escapeChar) {
      case '&': rv += "&amp;"; break;
      case '<': rv += "&lt;"; break;
      case '>': rv += "&gt;"; break;
      case '"': rv += "&quot;"; break;
      case '\'': rv += "&#x27;"; break;
      case '/': rv += "&#x2F;"; break;
      default: rv += escapeChar; break;
    }
  }
  return rv;
}

void handleRoot() {
  String rv;
  rv += "<!DOCTYPE html>";
  rv += "<html>";
  rv += "<head><title>Very simple file server</title></head>";
  rv += "<body>";
  rv += "<h1>Very simple file server</h1>";
  rv += "<p>This is a very simple file server to demonstrate the use of POST forms. </p>";
  rv += "<h2>List existing files</h2>";
  rv += "<p>See <a href=\"/public\">/public</a> to list existing files and retrieve or edit them.</p>";
  rv += "<h2>Upload new file</h2>";
  rv += "<p>This form allows you to upload files (text, jpg and png supported best). It demonstrates multipart/form-data.</p>";
  rv += "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">";
  rv += "file: <input type=\"file\" name=\"file\"><br>";
  rv += "<input type=\"submit\" value=\"Upload\">";
  rv += "</form>";
  rv += "</body>";
  rv += "</html>";
  server.send(200, "text/html", rv);
}

void handleFormEdit() {
  if (!server.authenticate("admin", "admin")) return server.requestAuthentication();
  bool hasFilename = server.hasArg("filename");;
	String filename = server.arg("filename");
	String pathname = String("/public/") + filename;
  if (server.hasArg("content") ){
    // Content has been edited. Save.
    File file = SPIFFS.open(pathname.c_str(), "w");
    String content = server.arg("content");
    file.write((uint8_t *)content.c_str(), content.length());
    file.close();
  }
  String content = server.arg("content");
	String rv;
    rv += "<html><head><title>Edit File</title><head><body>\n";
    File file = SPIFFS.open(pathname.c_str());
    if (!hasFilename) {
      rv += "<p>No filename specified.</p>\n";
    } else if (!file.available()) {
      rv += "<p>File not found:";
      rv += pathname;
      rv += "</p>\n";
    } else {
      rv += "<h2>Edit content of ";
      rv += pathname;
      rv += "</h2>\n";
      rv += "<form method=\"POST\" enctype=\"application/x-www-form-urlencoded\">\n";
      rv += "<input name=\"filename\" type=\"hidden\" value=\"";
      rv += filename;
      rv += "\">\n";
      rv += "<textarea name=\"content\" rows=\"24\" cols=\"80\">";
      // Read the file and write it to the response
      size_t length = 0;
      do {
        char buffer[256];
        length = file.read((uint8_t *)buffer, 256);
        std::string bufferString(buffer, length);
        bufferString = htmlEncode(bufferString);
        rv += String(bufferString.c_str());
      } while (length > 0);
      rv += "</textarea><br>";
      rv += "<input type=\"submit\" value=\"Save\">";
      rv += "</form>";
    }
    rv += "</body></html>";
    server.send(200, "text/html", rv);
}

void handleFormUpload() {
  Serial.println("handleUpload() called");
  server.send(200);
}

File fsUploadFile;

void handleFormUploadFile() {
  Serial.println("handleUploadFile() called");
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = String("/public/") + upload.filename;
    Serial.printf("upload filename=%s\n", filename.c_str());
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.printf("uploaded %d bytes\n", upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.printf("upload total %d bytes\n", upload.totalSize);
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/public");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  server.send(200, "text/plain", "OK");
  }
}

void handleDirectory() {
  String rv;
  rv += "<html><head><title>File Listing</title><head><body>\n";
  File d = SPIFFS.open("/public");
  if (!d.isDirectory()) {
    rv += "<p>No files found.</p>\n";
  } else {
    rv += "<h1>File Listing</h1>\n";
    rv += "<ul>\n";
    File f = d.openNextFile();
    while (f) {
      std::string pathname(f.name());
      std::string filename = pathname.substr(8); // Remove /public/
      rv += "<li><a href=\"";
      rv += String(pathname.c_str());
      rv += "\">";
      rv += String(filename.c_str());
      rv += "</a>";
      if (pathname.rfind(".txt") != std::string::npos) {
        rv += " <a href=\"/edit?filename=";
        rv += String(filename.c_str());
        rv += "\">[edit]</a>";
      }
      rv += "</li>";
      f = d.openNextFile();
    }
    rv += "</ul>";
  }
  rv += "</body></html>";
  server.send(200, "text/html", rv);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  // Setup filesystem
  if (!SPIFFS.begin(true)) Serial.println("Mounting SPIFFS failed");

  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, handleFormUpload, handleFormUploadFile);
  server.on("/edit", HTTP_GET, handleFormEdit);
  server.on("/edit", HTTP_POST, handleFormEdit);
  // Note: /public (without trailing /) gives directory listing, but /public/... retrieves static files.
  server.on("/public", HTTP_GET, handleDirectory);
  server.serveStatic("/public/", SPIFFS, "/public/");

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
