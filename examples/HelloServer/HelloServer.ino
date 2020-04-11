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
  line.toLowerCase();
  multi.toUpperCase();
  String rv;
  rv = "<html><head><title>Test Form</title></head><body>";
  rv += "<p>Single line will be converted to lowercase, multi line to uppercase. You can submit the form with three different methods.</p>";
  rv += "<h2>Form using GET</h2>";
  rv += "<form method='GET'>";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form>";
  rv += "<h2>Form using POST urlencoded</h2>";
  rv += "<form method='POST' action='/form'>";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form>";
  rv += "<h2>Form using POST with multipart</h2>";
  rv += "<form method='POST' enctype=\"multipart/form-data\">";
  rv += "Single line:<br><input name='line' value='" + line + "'><br>";
  rv += "Multi line:<br><textarea name='multi' rows='8' cols='40'>" + multi + "</textarea><br>";
  rv += "<input type='submit' value='upper+lower case'>";
  rv += "</form></body></html>";
  server.send(200, "text/html", rv);
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
