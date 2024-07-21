#include "server.h"
#include "motor.h"
#include "secrets.h"

WebSocketsServer webSocket = WebSocketsServer(81);
JsonDocument incomingDoc;
String tempData;

struct NetworkData {
  String ssid;
  String passphrase;
};

NetworkData networks[] = {
  {
    .ssid = WIFI_0_SSID ,
    .passphrase = WIFI_0_PASSPHRASE
  },
  {
    .ssid = WIFI_1_SSID,
    .passphrase = WIFI_1_PASSPHRASE
  }
};

uint8 numberOfNetworks = sizeof(networks) / sizeof(NetworkData);

bool SocketServer::send(uint8_t client, JsonDocument &doc){
  serializeJson(doc, tempData);
  return webSocket.sendTXT(client, tempData);
}

bool SocketServer::broadcast(JsonDocument &doc){
  serializeJson(doc, tempData);
  return webSocket.broadcastTXT(tempData);
}

void SocketServer::onWebSocketEvent(uint8_t clientId, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Motor::updateStatusDoc();
      break;
    case WStype_TEXT:
      deserializeJson(incomingDoc, (char *) payload, length);

      if(incomingDoc.containsKey("speed")){
        Motor::setSpeed(incomingDoc["speed"]);
      }

      if(incomingDoc.containsKey("program")){
        Motor::setProgram(incomingDoc["program"]);
      }
      break;
    case WStype_DISCONNECTED:
    case WStype_ERROR:
    case WStype_BIN:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PING:
    case WStype_PONG:
      break;
  }
}

void  SocketServer::setup(){
  for(uint8 t = 4; t > 0; t--) {
    delay(1000);
  }

  WiFi.mode(WiFiMode::WIFI_STA);
  WiFi.hostname("kuhlampia");

  uint8 currentNetworkIndex = 0;
  uint8 retries = 0;
  wl_status_t status;

  do {
    auto currentNetwork = networks[currentNetworkIndex];
    WiFi.begin(currentNetwork.ssid, currentNetwork.passphrase);

   do {
      delay(500);
      status = WiFi.status();
      retries++;
    } while (status != WL_CONNECTED && retries < 20);
    retries = 0;

    currentNetworkIndex++;
    if(currentNetworkIndex == numberOfNetworks) {
      currentNetworkIndex = 0;
    }
  } while (status != wl_status_t::WL_CONNECTED);

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void SocketServer::loop() {
  webSocket.loop();
}