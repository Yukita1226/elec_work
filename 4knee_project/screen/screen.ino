#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {

  float knee1;
  float knee2;
  float knee3;
  float knee4;

} struct_message;

struct_message knee;

void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  memcpy(&knee, data, sizeof(knee));

  Serial.print("From: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.printf("knee1: %.2f, knee2: %.2f, knee3: %.2f, knee4: %.2f\n",
                knee.knee1, knee.knee2, knee.knee3, knee.knee4);
}

void setup() {
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
}