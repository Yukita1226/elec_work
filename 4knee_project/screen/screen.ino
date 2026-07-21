#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "data.h"             
#include "html.h"

const char* ap_ssid = "KneeMonitor";

WebServer  server(80);
DNSServer  dnsServer;
Preferences prefs;
const byte DNS_PORT = 53;

Recive_data  knee;             
Setting_data settings;        

volatile unsigned long lastRecvTime = 0;
const unsigned long TIMEOUT_MS = 3000;


void loadSettings() {
  prefs.begin("knee", true);
  settings.theme    = (Theme)   prefs.getUChar("theme", dark);
  settings.language = (Language)prefs.getUChar("lang",  english);
  settings.metric   = (Modify)  prefs.getUChar("metric", gram);
  settings.isauto   =           prefs.getBool ("auto",  true);
  prefs.end();
}

void saveSettings() {
  prefs.begin("knee", false);
  prefs.putUChar("theme",  settings.theme);
  prefs.putUChar("lang",   settings.language);
  prefs.putUChar("metric", settings.metric);
  prefs.putBool ("auto",   settings.isauto);
  prefs.end();
}


void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(knee)) {
    memcpy(&knee, data, sizeof(knee));
    lastRecvTime = millis();
    Serial.printf("knee: %.2f %.2f %.2f %.2f\n",
                  knee.knee1, knee.knee2, knee.knee3, knee.knee4);
  }
}

bool isConnected() {
  return (lastRecvTime != 0) && (millis() - lastRecvTime < TIMEOUT_MS);
}

void handleRoot() {

  server.send(200, "text/html", PAGE);
}

void handleData() {
  String j = "{";
  j += "\"k\":[" + String(knee.knee1,2) + "," + String(knee.knee2,2) + ","
                 + String(knee.knee3,2) + "," + String(knee.knee4,2) + "],";
  j += "\"connected\":" + String(isConnected() ? "true" : "false") + ",";
  j += "\"theme\":"  + String((int)settings.theme)    + ",";
  j += "\"lang\":"   + String((int)settings.language) + ",";
  j += "\"metric\":" + String((int)settings.metric)   + ",";
  j += "\"auto\":"   + String(settings.isauto ? "true" : "false");
  j += "}";
  server.send(200, "application/json", j);
}

void handleSet() {
  if (server.hasArg("theme"))  settings.theme    = (Theme)   server.arg("theme").toInt();
  if (server.hasArg("lang"))   settings.language = (Language)server.arg("lang").toInt();
  if (server.hasArg("metric")) settings.metric   = (Modify)  server.arg("metric").toInt();
  if (server.hasArg("auto"))   settings.isauto   = (server.arg("auto").toInt() != 0);
  saveSettings();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  loadSettings();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, NULL, 1);
  Serial.print("AP IP: ");         Serial.println(WiFi.softAPIP());
  Serial.print("Receiver MAC: ");  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.on("/set",  handleSet);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}