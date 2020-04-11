# esp32\_https\_server\_compat

This library is a wrapper around the [TLS-enabled web server for the ESP32 using the Arduino core](https://github.com/fhessel/esp32_https_server), to make it compatible with the [default Webserver API](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer).

## Setup

The setup depends on the IDE that you're using.

### Using PlatformIO (recommended)

If you're using PlatformIO, just add `esp32\_https\_server\_compat` to the library depenendencies in your `platform.ini`:

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

To use the HTTPS server use `<ESPWebServerSecure.hpp>` and `ESPWebServerSecure` in stead of `ESPWebServer`.

More information and examples can be found in the default WebServer's [repository](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer). There are two minimal examples (more test programs, really) in the [examples](examples) directory.

## State of Development

The following issues are known:

- `serveStatic()` will serve only a single file or a single directory (as opposed to serving a whole subtree in the default WebServer).
- `serveStatic()` does not implement automatic gzip support.
- `serveStatic()` knows about only a limited set of mimetypes for file extensions.
- `authenticate()` and `requestAuthentication()` handle only `Basic` authentication, not `Digest` authentication.
- `sendHeader()` ignores the `first=true` parameter.
- `collectHeaders()` is not implemented.
- Handling of `POST` forms with mimetype `application/x-www-form-urlencoded` is memory-inefficient: the whole POST body is loaded into memory twice.