#include <esp_now.h>
#include <WiFi.h>


uint8_t receiverMac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

typedef struct struct_message {

  float knee1;
  float knee2;
  float knee3;
  float knee4;

} struct_message;

struct_message knee;

esp_now_peer_info_t peerInfo;


void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Pass" : "Fail");

}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP init failed");
    return;

  }

  esp_now_register_send_cb(onDataSent);


  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;

  }
}

void loop() {

  knee.knee1 = random(0,50000)/100.0;
  knee.knee2 = random(0,50000)/100.0;
  knee.knee3 = random(0,50000)/100.0;
  knee.knee4 = random(0,50000)/100.0;

  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&knee, sizeof(knee));

  if (result == ESP_OK) {
    Serial.println("Sent data successfully");
  } else {
    Serial.println("Error sending data");
  }

  delay(1000); 
}