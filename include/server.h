#ifndef IKEA_PS2014_WIFI_SERVER_H
#define IKEA_PS2014_WIFI_SERVER_H

#include "ArduinoJson.h"
#include "WebSocketsServer.h"

class SocketServer {
private:

  static void onWebSocketEvent(uint8_t clientId, WStype_t type, uint8_t * payload, size_t length);

public:
  static bool send(uint8_t client, JsonDocument &doc);
  static bool broadcast(JsonDocument &doc);

  static void setup();
  static void loop();
};



#endif //IKEA_PS2014_WIFI_SERVER_H
