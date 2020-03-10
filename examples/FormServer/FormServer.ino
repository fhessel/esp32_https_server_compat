#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#ifdef USE_DEFAULT_WEBSERVER
#include <WebServer.h>
#else
#include <ESPWebServer.hpp>
#endif
