#ifndef ABSTRACT_SERVER
#define ABSTRACT_SERVER

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Defines.h"
#include "AbstractState.h"
#include "AbstractStorage.h"


class Abstract_Server {
private:
  AsyncWebServer server;
  Abstract_State *state;
  Abstract_Storage *storage;
  void initWifi();
  void initServer();
public:
  Abstract_Server(Abstract_State *, Abstract_Storage *);
  void begin();
};

Abstract_Server::Abstract_Server(Abstract_State *state, Abstract_Storage *storage) : server(AsyncWebServer(80)) {
  this->state = state;
  this->storage = storage;
}

void Abstract_Server::begin() {
  initWifi();
  initServer();
  Serial.println("Server initialized");
}

void Abstract_Server::initWifi() {
  WiFi.softAP(APSSID, PASSWORD);
  IPAddress IPAddr = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IPAddr);
}

void Abstract_Server::initServer() {
  // Basic responses defined

  // Route for root / web page from the onboard file system
  this->server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Main page request");
    request->send(SPIFFS, "/main.html");
  });
  // Sends the styles sheet from the datasystem
  this->server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("CSS request");
    request->send(SPIFFS, "/styles.css");

  });
  // Sends the graphing script from the datasystem
  this->server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("JS request");
    request->send(SPIFFS, "/script.js");

  });
  // Responds with current server time
  this->server.on("/time", HTTP_GET, [this](AsyncWebServerRequest *request){
     Serial.println("Time request");
    // Sends the general time as rtc.now() calls require use the delay function
    request->send(200, "text/plain", String(this->state->rtc->tnow().unixtime()));
  });
  // Minimal Error response
  this->server.onNotFound([](AsyncWebServerRequest *request){
    Serial.println("404 response");
    request->send(404);
  });
  this->server.on("/currData", HTTP_GET, [this](AsyncWebServerRequest *request){
    Serial.println("Current Data request");
    request->send(200, "text/plain", this->state->getDataString());
  });
  this->server.on("/oldData", HTTP_GET, [this](AsyncWebServerRequest *request){
    Serial.println("Old Data request");
    if (error & SDERROR) { request->send(404); return;}
    AsyncWebHeader* h = request->getHeader("subMonth");
    int sub = String(h->value()).toInt();
    String filename = dataFileName(this->state->rtc->tnow()-TimeSpan(sub,0,0,0));
    request->send(this->storage->returnDatalogFile(this->state,sub),filename);
  });
  this->server.on("/getSettings", HTTP_GET, [this](AsyncWebServerRequest *request){
    Serial.println("Get Settings request");
    request->send(200, "text/plain", this->state->getSettingsString());
  });
  this->server.on("/setSettings", HTTP_POST, [this](AsyncWebServerRequest *request){
    Serial.println("Set settings request");
    AsyncWebHeader* humSetH = request->getHeader("Hum");
    AsyncWebHeader* co2SetH = request->getHeader("Co2");
    AsyncWebHeader* f1f2H = request->getHeader("F1f2");
    float humSet = humSetH->value().toFloat();
    float co2Set = co2SetH->value().toFloat();
    float f1f2 = f1f2H->value().toFloat();

    if ((humSet>=0) && (humSet <= 100)) { this->state->humiditySetPoint = humSet; }
    if ((co2Set>=400) && (co2Set <= 5000)) { this->state->co2SetPoint = co2Set; }
    if ((f1f2>=0) && (f1f2 <= 5000))  { this->state->f1Withf2 = f1f2; }

    this->storage->setSettings(this->state);
    Serial.println(this->state->getSettingsString());
    request->send(200, "text/plain", this->state->getSettingsString());
  });

  this->server.begin();
}
#endif
