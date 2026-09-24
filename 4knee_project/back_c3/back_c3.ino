#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

uint8_t receiverMac[] = {0x14, 0x2B, 0x2F, 0xC0, 0xDB, 0x14};
const int k[4] = {1,2,3,4};

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

  uint8_t prim; wifi_second_chan_t sec;
  esp_wifi_get_channel(&prim, &sec);
  Serial.printf("receiver channel: %d\n", prim);

  


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

  knee.knee1 = analogRead(k[0])*100.0/4095.0;
  knee.knee2 = analogRead(k[1])*100.0/4095.0;
  knee.knee3 = analogRead(k[2])*100.0/4095.0;
  knee.knee4 = analogRead(k[3])*100.0/4095.0;

  // knee.knee1 = random(0,100);
  // knee.knee2 = random(0,100);
  // knee.knee3 = random(0,100);
  // knee.knee4 = random(0,100);

  int kn[4] = {knee.knee1,knee.knee2,knee.knee3,knee.knee4};

  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&knee, sizeof(knee));

  if (result == ESP_OK) {
    Serial.println("Sent data successfully");

  } else {
    Serial.println("Error sending data");
  }

  Serial.print("test knee  :");
  for (int a  =0 ; a< 4; a++) {
    Serial.print(kn[a]);
    Serial.print(" , ");
  }
  Serial.println("");



  delay(1000); 
}