#define WEBSOCKETS_NETWORK_TYPE 0 // NETWORK_ESP8266_ASYNC

#include <Arduino.h>
#include "motor.h"
#include "server.h"

void setup() {
  SocketServer::setup();
  Motor::setup();
}

void loop() {
  SocketServer::loop();
  Motor::loop();
}