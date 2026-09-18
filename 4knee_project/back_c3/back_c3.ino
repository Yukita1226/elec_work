#include <esp_now.h>
#include <WiFi.h>


uint8_t receiverMac[] = {0x84, 0x1F, 0xE8, 0x1C, 0x6A, 0x98};
const int test = 4;

typedef struct struct_message {

  float knee1;
  float knee2;
  float knee3;
  float knee4;

} struct_message;

struct_message knee;

esp_now_peer_info_t peerInfo;


void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 
  analogReadResolution(12);             
  analogSetAttenuation(ADC_11db);

  delay(2000);
  Serial.println(" ok");

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

  knee.knee1 = analogRead(test)*3.3/4095.0;
  knee.knee2 = random(0,50000)/100.0;
  knee.knee3 = random(0,50000)/100.0;
  knee.knee4 = random(0,50000)/100.0;

  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&knee, sizeof(knee));

  if (result == ESP_OK) {
    Serial.println("Sent data successfully");

    Serial.print("test knee 1 :");
  } else {
    Serial.println("Error sending data");
  }

  Serial.println(knee.knee1);

  delay(1000); 
}