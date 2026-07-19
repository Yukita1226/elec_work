#include <MQ131.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  float o3_ppm;
  int pm1_0;
  int pm2_5;
  int pm10;
  uint8_t aqi;
  uint16_t tvoc;
  uint16_t eco2;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

#define I2C_SDA 27
#define I2C_SCL 26
#define PMS_RX 16
#define PMS_TX 17
#define MQ131_PIN 34

HardwareSerial pmsSerial(2);   
uint8_t ensAddress = 0x52; 

unsigned long lastReadTime = 0;
const int readInterval = 3000; 

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Air Delivery: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(2, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 2;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Wire.begin(I2C_SDA, I2C_SCL);
  
  Wire.beginTransmission(ensAddress);
  Wire.write(0x10); 
  Wire.write(0x02); 
  Wire.endTransmission();

  pmsSerial.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
}

void loop() {
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    readSensors();
  }
}

void readSensors() {
  myData.pm1_0 = 0; myData.pm2_5 = 0; myData.pm10 = 0;
  myData.aqi = 0; myData.tvoc = 0; myData.eco2 = 0;

  int rawO3 = analogRead(MQ131_PIN);
  myData.o3_ppm = (rawO3 / 4095.0) * 3.3;

  while (pmsSerial.available() >= 32) {
    if (pmsSerial.peek() == 0x42) {
      if (pmsSerial.read() == 0x42 && pmsSerial.read() == 0x4D) {
        uint8_t buffer[30];
        pmsSerial.readBytes(buffer, 30);
        myData.pm1_0 = (buffer[8] << 8) | buffer[9];
        myData.pm2_5 = (buffer[10] << 8) | buffer[11];
        myData.pm10  = (buffer[12] << 8) | buffer[13];
        break;
      }
    } else {
      pmsSerial.read();
    }
  }

  Wire.beginTransmission(ensAddress);
  Wire.write(0x21); 
  if (Wire.endTransmission(false) == 0) {
    Wire.requestFrom((uint16_t)ensAddress, (uint8_t)6, true);
    if (Wire.available() == 6) {
      myData.aqi = Wire.read();
      uint8_t t_lsb = Wire.read(); uint8_t t_msb = Wire.read();
      uint8_t e_lsb = Wire.read(); uint8_t e_msb = Wire.read();
      myData.tvoc = (t_msb << 8) | t_lsb;
      myData.eco2 = (e_msb << 8) | e_lsb;
    }
  }

  Serial.println("--- DATA OUTPUT ---");
  Serial.print("O3 Volts: "); Serial.println(myData.o3_ppm);
  Serial.print("PM2.5: "); Serial.println(myData.pm2_5);
  Serial.print("AQI: "); Serial.println(myData.aqi);
  Serial.print("TVOC: "); Serial.println(myData.tvoc);
  Serial.print("eCO2: "); Serial.println(myData.eco2);

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}