#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#ifdef USE_DEFAULT_WEBSERVER
#include <WebServer.h>
typedef WebServer ESPWebServer;
#else
#include <ESPWebServer.hpp>
#endif

const char* ssid = "........";
const char* password = "........";

ESPWebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32! See /form and /inline too!");
  digitalWrite(led, 0);
}

void handleForm() {
  String line = server.arg("line");
  Serial.print("line: ");
  Serial.println(line);
  String multi = server.arg("multi");
  Serial.print("multi: ");
  Serial.println(multi);
  String file = server.arg("file");
  Serial.print("file: ");
  Serial.println(file);
  line.toLowerCase();
  multi.toUpperCase();
  String rv;
  rv = "<html><head><title>Test Form</title></head><body>";
  rv += "<h2>Form using GET</h2>";
  rv += "<form method='GET'>";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "File:<br><input type='file' name='file'><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form></body></html>";
  rv += "<h2>Form using POST urlencoded</h2>";
  rv += "<form method='POST' action='/form'>";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "File:<br><input type='file' name='file'><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form></body></html>";
  rv += "<h2>Form using POST with multipart</h2>";
  rv += "<form method='POST' action=\"/upload\" enctype=\"multipart/form-data\">";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "File:<br><input type='file' name='file'><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form></body></html>";
  server.send(200, "text/html", rv);
}

void handleUpload() {
  Serial.println("handleUpload() called");
  String line = server.arg("line");
  Serial.print("line: ");
  Serial.println(line);
  String multi = server.arg("multi");
  Serial.print("multi: ");
  Serial.println(multi);
  server.send(200);
}

void handleUploadFile() {
  Serial.println("handleUploadFile() called");
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    Serial.printf("upload filename=%s\n", filename.c_str());
#if 0
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
#endif
  } else if(upload.status == UPLOAD_FILE_WRITE){
    Serial.printf("uploaded %d bytes\n", upload.currentSize);
#if 0
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
#endif
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.printf("upload total %d bytes\n", upload.totalSize);
#if 0
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
#endif
  server.send(200, "text/plain", "OK");
  }
}

void handleNotFound() {
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
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

  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.on("/form", HTTP_POST, handleForm);
  server.on("/upload", HTTP_POST, handleUpload, handleUploadFile);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
