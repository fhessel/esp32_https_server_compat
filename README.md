# esp32_https_server_compat

This library is a wrapper around the [TLS-enabled web server for the ESP32 using the Arduino core](https://github.com/fhessel/esp32_https_server), to make it compatible with the [default Webserver API](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer).

## Setup

The setup depends on the IDE that you're using.

### Using PlatformIO (recommended)

If you're using PlatformIO, just add esp32_https_server_compat to the library depenendencies in your platform.ini:

```ini
[env:myenv]
lib_deps = esp32_https_server_compat
```

### Using the Arduino IDE

Download this library from the [releases tab](https://github.com/fhessel/esp32_https_server_compat/releases) on GitHub and put it into the `libraries` folder of your Arduino installation. In addition, you need the underlying `esp32_https_server` library. Download the _same version_ from [that repository's releases tab](https://github.com/fhessel/esp32_https_server/releases) and also put it into the `libraries` folder besides the previously downloaded library. Restart your IDE if it is currently running.

## Usage

The library can be used in the same way as the default WebServer library, with the only difference that you need to include `ESPWebServer.hpp` instead of `WebServer.h`:

```c++
#include <ESPWebServer.hpp>

ESPWebserver server(80);

void setup() {
  server.begin();
}

void loop() {
  server.handleClient();
}
```

More information and examples can be found in the default WebServer's [repository](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer).

## State of Development

This wrapper is still very WIP.

| Function | State | Comment |
| -------- | ----- | ------- |
| Starting and stopping the server | ✅ | (but not tested) |
| Handling basic requests | ✅ | `on(...)` |
| Handling 404 | ✅ | `onNotFound(...)` |
| Providing access to request properties | ✅ | `uri()`, `method()` |
| Handling file uploads | ❌ | `onFileUpload(...)`, `upload()`, and `on()` with 4 parameters |
| Handling headers | ❌ | `header()`, `headerName()`, `headers()` etc. |
| Handling arguments | ❌ | `arg()`, `argName()`, `hasArg()` etc. |
| Handling forms | ❌ | Needs [esp32_https_server#29](https://github.com/fhessel/esp32_https_server/issues/29) first. |
| Sending responses | ❌ | `send()` etc. |
| CORS and cross-origin | ❌ | Needs headers first |
| Streaming files | ❌ | `streamFile()` |
| `FS` support | ❌ | |
| TLS | ❌ | Needs `ESPWebServerSecure` that extends `ESPWebServer` |
