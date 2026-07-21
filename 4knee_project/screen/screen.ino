#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>


const char* ap_ssid = "KneeMonitor";

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

typedef struct struct_message {
  float knee1;
  float knee2;
  float knee3;
  float knee4;
} struct_message;

struct_message knee;

void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  memcpy(&knee, data, sizeof(knee));

  Serial.printf("knee1: %.2f, knee2: %.2f, knee3: %.2f, knee4: %.2f\n",
                knee.knee1, knee.knee2, knee.knee3, knee.knee4);
}


void handleRoot() {
  String html = "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<meta http-equiv='refresh' content='1'>"  
    "<title>Knee Monitor</title>"
    "<style>"
    "body{font-family:sans-serif;text-align:center;background:#111;color:#eee;}"
    ".card{background:#222;margin:10px auto;padding:20px;border-radius:12px;max-width:300px;}"
    ".val{font-size:2em;color:#4caf50;}"
    "</style></head><body>"
    "<h1>Knee Sensor Data</h1>";

  html += "<div class='card'>Knee 1<div class='val'>" + String(knee.knee1, 2) + "</div></div>";
  html += "<div class='card'>Knee 2<div class='val'>" + String(knee.knee2, 2) + "</div></div>";
  html += "<div class='card'>Knee 3<div class='val'>" + String(knee.knee3, 2) + "</div></div>";
  html += "<div class='card'>Knee 4<div class='val'>" + String(knee.knee4, 2) + "</div></div>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}


void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, NULL, 1); 

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());  
  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);


  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}