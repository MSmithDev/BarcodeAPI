# barcodeapi-esp32

Lightweight C++ client for ESP32 devices that works in both Arduino and ESP-IDF
projects.  It exposes all [BarcodeAPI](https://barcodeapi.org) features, from
barcode generation and decoding to bulk operations and metadata queries.  The
client is transport-agnostic but provides default HTTP implementations for
Arduino's `HTTPClient` and IDF's `esp_http_client`.

## Installation

Copy the `barcodeapi_esp32` directory into your Arduino sketchbook or include it
in your ESP-IDF component list.

## Usage

### Arduino

```cpp
#include <WiFi.h>
#include "barcodeapi_esp32/barcodeapi.h"

BarcodeAPI api("https://barcodeapi.org", "my-token");

void setup() {
  WiFi.begin(ssid, pass); // connect to WiFi
  // ... wait for connection ...
  auto img = api.generate("hello world", "qr");
  Serial.println(img.body.c_str());
}

void loop() {}
```

### ESP-IDF

```cpp
#include "barcodeapi_esp32/barcodeapi.h"

extern "C" void app_main() {
  BarcodeAPI api("https://barcodeapi.org");
  auto info = api.getInfo();
  printf("%s\n", info.body.c_str());
}
```

## Available Methods

- `generate(data, codeType="auto", params={}, headers={})`
- `decode(image)`
- `bulkGenerate(csv)`
- `getInfo()`
- `getTypes()`
- `getType(typeName)`
- `getLimiter()`
- `getSession()`
- `deleteSession()`
- `createShare(requestsList)`
- `getShare(key)`
- `setToken(token)`

The constructor accepts an optional requester function for custom transports
(useful for testing) and a token used for authentication.

