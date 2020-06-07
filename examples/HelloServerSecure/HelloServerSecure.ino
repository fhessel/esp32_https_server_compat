/**
 * This example shows how you use the server with a secure TLS connection.
 * 
 * It is basically the same code as the HelloServer example, and changes
 * are marked with an "TLS" prefix in the comment.
 * 
 * To run the example, you need a certificate and a private key. If you're
 * not familiar with these terms, you may need to look up how Transport
 * Layer Security (TLS) works. Basically you have two keys, a public and
 * a private key. Simplified, you can encrypt data with one of the keys and
 * only decrypt it with the _other_ key. So while the public key can be
 * shared with everybody (therefore public), and it is part of the certificate,
 * it is crucial that the private keys remains only on the server.
 * For this very reason you MUST NOT USE THE KEYS PROVIDED WITH THIS EXAMPLE
 * for any of your projects. No, not even private ones in your own network.
 * Using this example key gives you the same level of security that you would
 * have without encryption. You have been warned.
 * 
 * While we include the keys from two additional files "cert.h" and
 * "private_key.h", you are free to store the key whereever you want, e.g. in
 * the SPIFFS file system. You only need to be able to load it into a byte
 * array and pass it to the web server.
 * 
 * All keys have to be in the DER format, and only RSA keys are supported.
 */

// Header file for WiFi
#include <WiFi.h>

// TLS: Header file for the server. Note that we use the ...Secure.hpp version
#include <ESPWebServerSecure.hpp>

// TLS: We need the private key and certificate from the external files
// WARNING: NEVER USE THE FILES FROM THE EXAMPLE FOR MORE THAN TESTING.
//          AS THE PRIVATE KEY IS PART OF THE REPOSITORY, THIS MEANS YOU
//          HAVE BASICALLY THE SAME LEVEL OF SECURITY AS WITH PLAIN
//          HTTP.
//          See the repository of the underlying esp32_https_server for
//          details on how to generate your own:
//          https://github.com/fhessel/esp32_https_server/tree/master/extras#create_certsh
#include "cert.h"
#include "private_key.h"

// TODO: Configure your WiFi here
const char* ssid = ".............";
const char* password = "......................";

ESPWebServerSecure server(443);

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

  // TLS: Before configuring the server, you need to set the certificate
  server.setServerKeyAndCert(
    example_key_der,     // Raw DER key data as byte array
    example_key_der_len, // Length of the key array
    example_der,     // Raw DER certificate (no certificate chain!) as byte array
    example_der_len  // Length of the certificate array
  );

  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.on("/form", HTTP_POST, handleForm);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");
}

void loop(void) {
  server.handleClient();
}
