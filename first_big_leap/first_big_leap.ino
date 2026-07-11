#include "lgfx_user_setup.h"
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Preferences.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define GT911_INT_PIN 4
#define RS485_RX 15
#define RS485_TX 16

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define GRAY    0x4208
#define DKGRAY  0x2104
#define ORANGE  0xFD20
#define MAGENTA 0xF81F

#define SYNC_BTN_X 300
#define SYNC_BTN_Y 390
#define SYNC_BTN_W 200
#define SYNC_BTN_H 60

#define MAX_GRAPH_POINTS 70

typedef struct struct_message {
  float o3_ppm;
  int pm1_0;
  int pm2_5;
  int pm10;
  uint8_t aqi;
  uint16_t tvoc;
  uint16_t eco2;
} struct_message;

struct_message incomingData;
bool newData = false;

bool relayState[32] = {false};
uint32_t relayBitmask = 0;
Preferences prefs;
uint32_t lastTouchRead = 0;

uint8_t currentTab = 0; 
float graphPM1[MAX_GRAPH_POINTS];
float graphPM25[MAX_GRAPH_POINTS];
float graphPM10[MAX_GRAPH_POINTS];
float graphO3[MAX_GRAPH_POINTS];
float graphVOC[MAX_GRAPH_POINTS];
float graphCO2[MAX_GRAPH_POINTS];
float graphAQI[MAX_GRAPH_POINTS];

int graphCount = 0;
int graphHead = 0;

void drawSingleButton(int i);
void drawTabBar();
void drawTab1();
void drawTab2();
void drawTab3();
void updateTab1Data();
void drawSyncButton(bool pressed);

uint16_t getCRC(uint8_t *buf, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void controlRelay(int relayNumber, bool state) {
  uint16_t address = relayNumber;
  uint16_t value = state ? 256 : 512;
  uint8_t frame[8];
  frame[0] = 0x02;
  frame[1] = 0x06;
  frame[2] = (address >> 8) & 0xFF;
  frame[3] = address & 0xFF;
  frame[4] = (value >> 8) & 0xFF;
  frame[5] = value & 0xFF;
  uint16_t crc = getCRC(frame, 6);
  frame[6] = crc & 0xFF;
  frame[7] = (crc >> 8) & 0xFF;
  Serial2.write(frame, 8);
  Serial2.flush();
  uint32_t escapeTimer = millis();
  while (Serial2.available() && (millis() - escapeTimer < 20)) {
    Serial2.read();
  }
}

void requestRelayStatus() {
  uint8_t frame[8];
  frame[0] = 0x02;
  frame[1] = 0x01;
  frame[2] = 0x00;
  frame[3] = 0x00;
  frame[4] = 0x00;
  frame[5] = 0x20;
  uint16_t crc = getCRC(frame, 6);
  frame[6] = crc & 0xFF;
  frame[7] = (crc >> 8) & 0xFF;
  Serial2.write(frame, 8);
  Serial2.flush();
}

void checkModbusFeedback() {
  while (Serial2.available() >= 9) {
    if (Serial2.peek() != 0x02) {
      Serial2.read();
      continue;
    }
    uint8_t resp[9];
    for (int i = 0; i < 9; i++) {
      resp[i] = Serial2.read();
    }
    if (resp[1] != 0x01 || resp[2] != 0x04) continue;
    uint16_t crc_calc = getCRC(resp, 7);
    uint16_t crc_recv = resp[7] | (resp[8] << 8);
    if (crc_calc != crc_recv) continue;
    uint32_t hardwareBitmask = ((uint32_t)resp[6] << 24) |
                               ((uint32_t)resp[5] << 16) |
                               ((uint32_t)resp[4] << 8)  |
                               ((uint32_t)resp[3]);
    if (hardwareBitmask != relayBitmask) {
      uint32_t oldBitmask = relayBitmask;
      relayBitmask = hardwareBitmask;
      prefs.putUInt("relays", relayBitmask);
      for (int i = 0; i < 32; i++) {
        relayState[i] = (relayBitmask >> i) & 1;
      }
      uint32_t changed = oldBitmask ^ relayBitmask;
      for (int i = 0; i < 32; i++) {
        if (changed & (1UL << i)) {
          drawSingleButton(i);
        }
      }
    }
    return;
  }
}

void loadRelayStateFromPrefs() {
  relayBitmask = prefs.getUInt("relays", 0);
  for (int i = 0; i < 32; i++) {
    relayState[i] = (relayBitmask >> i) & 1;
  }
}

void syncRelayBoardFromSavedState() {
  for (int i = 0; i < 32; i++) {
    controlRelay(i + 1, relayState[i]);
    delay(20);
  }
}

void recordGraphData() {
  graphPM1[graphHead]  = (float)incomingData.pm1_0;
  graphPM25[graphHead] = (float)incomingData.pm2_5;
  graphPM10[graphHead] = (float)incomingData.pm10;
  graphO3[graphHead]   = incomingData.o3_ppm;
  graphVOC[graphHead]  = (float)incomingData.tvoc;
  graphCO2[graphHead]  = (float)incomingData.eco2;
  graphAQI[graphHead]  = (float)incomingData.aqi;
  
  graphHead = (graphHead + 1) % MAX_GRAPH_POINTS;
  if(graphCount < MAX_GRAPH_POINTS) graphCount++;
}

static const uint8_t CH422G_ADDR_WR_SET = 0x24;
static const uint8_t CH422G_ADDR_WR_IO  = 0x38;
static const uint8_t EXIO_TP_RST = 1;
static const uint8_t EXIO_DISP   = 2;
static uint8_t g_exio_state = 0xFF;

bool i2cWrite1(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(data);
  return (Wire.endTransmission() == 0);
}
bool ch422_set_exio_write_mode() { return i2cWrite1(CH422G_ADDR_WR_SET, 0x01); }
bool ch422_write_exio_state() { return i2cWrite1(CH422G_ADDR_WR_IO, g_exio_state); }
bool ch422_exio_write_pin(uint8_t exio_pin, bool high_release) {
  if (high_release) g_exio_state |= (1 << exio_pin);
  else g_exio_state &= ~(1 << exio_pin);
  if (!ch422_set_exio_write_mode()) return false;
  return ch422_write_exio_state();
}

void gt911_reset_select_addr(uint8_t desiredAddr) {
  pinMode(GT911_INT_PIN, OUTPUT);
  digitalWrite(GT911_INT_PIN, (desiredAddr == 0x14) ? HIGH : LOW);
  ch422_exio_write_pin(EXIO_TP_RST, false);
  delay(10);
  ch422_exio_write_pin(EXIO_TP_RST, true);
  delay(5);
  digitalWrite(GT911_INT_PIN, LOW);
  delay(50);
  pinMode(GT911_INT_PIN, INPUT);
  delay(50);
}

uint8_t gt911_probe_addr() {
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) return 0x5D;
  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) return 0x14;
  return 0;
}

struct TP_Point { uint8_t id; uint16_t x; uint16_t y; uint16_t size; };
enum { ROTATION_LEFT = 0, ROTATION_INVERTED = 1, ROTATION_RIGHT = 2, ROTATION_NORMAL = 3 };

class WaveshareGT911 {
public:
  bool isTouched = false;
  uint8_t touches = 0;
  TP_Point points[5];
  WaveshareGT911(uint16_t w, uint16_t h) : width(w), height(h) {}
  void setRotation(uint8_t r) { rotation = r; }
  bool begin() {
    ch422_set_exio_write_mode();
    ch422_exio_write_pin(EXIO_DISP, true);
    gt911_reset_select_addr(0x5D);
    addr = gt911_probe_addr();
    if (addr == 0x5D) return true;
    gt911_reset_select_addr(0x14);
    addr = gt911_probe_addr();
    return (addr == 0x14);
  }
  void read() {
    isTouched = false;
    touches = 0;
    uint8_t pointInfo = readByte(0x814E);
    uint8_t bufferStatus = (pointInfo >> 7) & 0x01;
    touches = pointInfo & 0x0F;
    if (bufferStatus && touches > 0) {
      if (touches > 5) touches = 5;
      for (uint8_t i = 0; i < touches; i++) {
        uint8_t data[7];
        readBlock(0x814F + i * 8, data, 7);
        points[i].id = data[0];
        points[i].x = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
        points[i].y = (uint16_t)data[3] | ((uint16_t)data[4] << 8);
        points[i].size = (uint16_t)data[5] | ((uint16_t)data[6] << 8);
        mapPoint(points[i].x, points[i].y);
      }
      isTouched = true;
    }
    writeByte(0x814E, 0);
  }
private:
  uint16_t width, height;
  uint8_t addr = 0;
  uint8_t rotation = ROTATION_NORMAL;
  void mapPoint(uint16_t &x, uint16_t &y) {
    uint16_t t;
    switch (rotation) {
      case ROTATION_NORMAL: x = width - x; y = height - y; break;
      case ROTATION_LEFT: t = x; x = width - y; y = t; break;
      case ROTATION_INVERTED: break;
      case ROTATION_RIGHT: t = x; x = y; y = height - t; break;
    }
  }
  void writeByte(uint16_t reg, uint8_t val) {
    if (!addr) return;
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8); Wire.write(reg & 0xFF); Wire.write(val);
    Wire.endTransmission();
  }
  uint8_t readByte(uint16_t reg) {
    if (!addr) return 0;
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8); Wire.write(reg & 0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom((int)addr, 1);
    return Wire.available() ? Wire.read() : 0;
  }
  void readBlock(uint16_t reg, uint8_t *buf, uint8_t len) {
    if (!addr) return;
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8); Wire.write(reg & 0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom((int)addr, (int)len);
    for (uint8_t i = 0; i < len; i++) {
      buf[i] = Wire.available() ? Wire.read() : 0;
    }
  }
};

WaveshareGT911 tp(800, 480);
LGFX lcd;
LGFX* display = &lcd;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataRaw, int len) {
  if (len != sizeof(struct_message)) return;
  memcpy(&incomingData, incomingDataRaw, sizeof(struct_message));
  newData = true;
}

void drawTabBar() {
  display->fillRect(0, 0, 800, 40, DKGRAY);
  display->fillRect(0, 0, 266, 40, currentTab == 0 ? BLUE : DKGRAY);
  display->fillRect(266, 0, 266, 40, currentTab == 1 ? BLUE : DKGRAY);
  display->fillRect(532, 0, 268, 40, currentTab == 2 ? BLUE : DKGRAY);
  display->setTextColor(WHITE);
  display->setTextSize(2); // set size here
  display->setCursor(100, 12); display->print("DATA");
  display->setCursor(360, 12); display->print("CHART");
  display->setCursor(630, 12); display->print("CTRL");
  display->drawFastVLine(266, 0, 40, BLACK);
  display->drawFastVLine(532, 0, 40, BLACK);
  display->drawFastHLine(0, 40, 800, WHITE);
}


void drawTab1() { // this is where i dix
  display->fillRect(0, 41, 800, 439, BLACK);

  display->drawRect(40, 60, 340, 180, CYAN);
  display->fillRect(40, 60, 340, 30, CYAN);
  display->setTextColor(BLACK);
  display->setTextSize(3); // set size here
  display->setCursor(150, 68);
  display->print("PARTICULATES");

  display->drawRect(420, 60, 340, 180, CYAN);
  display->fillRect(420, 60, 340, 30, CYAN);
  display->setTextColor(BLACK);
  display->setTextSize(3);
  display->setCursor(560, 68);
  display->print("GASES");

  display->drawRect(250, 260, 300, 160, GREEN);
  display->fillRect(250, 260, 300, 30, GREEN);
  display->setTextColor(BLACK);
  display->setTextSize(3);
  display->setCursor(305, 268);
  display->print("AQI INDEX");

  display->setTextColor(WHITE);
  display->setTextSize(3);
  display->setCursor(60, 110);  display->print("PM1  :");
  display->setCursor(60, 150);  display->print("PM2.5:");
  display->setCursor(60, 190);  display->print("PM10 :");

  display->setCursor(440, 110); display->print("O3   :");
  display->setCursor(440, 150); display->print("VOC  :");
  display->setCursor(440, 190); display->print("CO2  :");

  display->setTextSize(4); // fix size here
  display->setTextColor(GREEN, BLACK);
  display->setCursor(345, 335);

  if (incomingData.aqi == 1) display->print("VERY GOOD");
  else if (incomingData.aqi == 2) display->print("GOOD");
  else if (incomingData.aqi == 3) display->print("OK");
  else if (incomingData.aqi == 4) display->print("POOR");
  else if (incomingData.aqi == 5) display->print("BAD");
  else display->print("N/A");

  display->setTextSize(4); // fix size here
  display->setTextColor(WHITE);

  display->setCursor(160, 110); display->printf("%-6d", incomingData.pm1_0);
  display->setCursor(160, 150); display->printf("%-6d", incomingData.pm2_5);
  display->setCursor(160, 190); display->printf("%-6d", incomingData.pm10);

  display->setCursor(540, 110); display->printf("%-6.2f", incomingData.o3_ppm);
  display->setCursor(540, 150); display->printf("%-6d", incomingData.tvoc);
  display->setCursor(540, 190); display->printf("%-6d", incomingData.eco2);
}



void updateTab1Data() {
  if (currentTab != 0) return;

  display->setTextSize(2);
  display->setTextColor(WHITE, BLACK);
  display->setCursor(160, 110); display->printf("%-6d", incomingData.pm1_0);
  display->setCursor(160, 150); display->printf("%-6d", incomingData.pm2_5);
  display->setCursor(160, 190); display->printf("%-6d", incomingData.pm10);
  display->setCursor(540, 110); display->printf("%-6.2f", incomingData.o3_ppm);
  display->setCursor(540, 150); display->printf("%-6d", incomingData.tvoc);
  display->setCursor(540, 190); display->printf("%-6d", incomingData.eco2);

  display->fillRect(270, 320, 260, 40, BLACK);
  display->setTextSize(3);
  display->setTextColor(GREEN, BLACK);
  display->setCursor(280, 330);

  if (incomingData.aqi == 1) display->print("VERY GOOD");
  else if (incomingData.aqi == 2) display->print("GOOD");
  else if (incomingData.aqi == 3) display->print("OK");
  else if (incomingData.aqi == 4) display->print("POOR");
  else if (incomingData.aqi == 5) display->print("BAD");
  else display->print("N/A");
}


void drawTab2() {
  display->fillRect(0, 41, 800, 439, BLACK);
  display->drawLine(60, 450, 560, 450, WHITE);
  display->drawLine(60, 50, 60, 450, WHITE);
  display->fillRect(570, 50, 210, 160, DKGRAY);
  display->drawRect(570, 50, 210, 160, WHITE);
  display->setTextSize(2);
  display->setTextColor(WHITE, DKGRAY);   display->setCursor(580, 60);  display->printf("PM1.0: %-5d", (int)incomingData.pm1_0);
  display->setTextColor(RED, DKGRAY);     display->setCursor(580, 80);  display->printf("PM2.5: %-5d", (int)incomingData.pm2_5);
  display->setTextColor(GREEN, DKGRAY);   display->setCursor(580, 100); display->printf("PM10 : %-5d", (int)incomingData.pm10);
  display->setTextColor(CYAN, DKGRAY);    display->setCursor(580, 120); display->printf("O3   : %-5.2f", incomingData.o3_ppm);
  display->setTextColor(YELLOW, DKGRAY);  display->setCursor(580, 140); display->printf("VOC  : %-5d", (int)incomingData.tvoc);
  display->setTextColor(ORANGE, DKGRAY);  display->setCursor(580, 160); display->printf("CO2  : %-5d", (int)incomingData.eco2);
  display->setTextColor(MAGENTA, DKGRAY); display->setCursor(580, 180); display->printf("AQI  : %-5d", (int)incomingData.aqi);

  if (graphCount < 2) return;

  float max_v = 1.0f, min_v = 1000000.0f;
  for (int i = 0; i < graphCount; i++) {
    float vals[] = {graphPM1[i], graphPM25[i], graphPM10[i], graphO3[i], graphVOC[i], graphCO2[i], graphAQI[i]};
    for(float v : vals) {
      if(v > max_v) max_v = v;
      if(v < min_v) min_v = v;
    }
  }
  float range = max_v - min_v;
  if (range == 0) range = 1.0f;

  display->setTextSize(2);
  display->setTextColor(WHITE, BLACK);
  display->setCursor(5, 50);  display->printf("%4d", (int)max_v);
  display->setCursor(5, 245); display->printf("%4d", (int)((max_v + min_v) / 2));
  display->setCursor(5, 434); display->printf("%4d", (int)min_v);

  int stepX = 500 / MAX_GRAPH_POINTS;
  for (int i = 0; i < graphCount - 1; i++) {
    int idx1 = (graphHead - graphCount + i + MAX_GRAPH_POINTS) % MAX_GRAPH_POINTS;
    int idx2 = (idx1 + 1) % MAX_GRAPH_POINTS;
    int x1 = 60 + i * stepX, x2 = 60 + (i + 1) * stepX;
    auto drawLineVal = [&](float g[], uint16_t c) {
      int y1 = 450 - (int)((g[idx1] - min_v) / range * 400);
      int y2 = 450 - (int)((g[idx2] - min_v) / range * 400);
      display->drawLine(x1, y1, x2, y2, c);
    };
    drawLineVal(graphPM1, WHITE); drawLineVal(graphPM25, RED); drawLineVal(graphPM10, GREEN);
    drawLineVal(graphO3, CYAN); drawLineVal(graphVOC, YELLOW); drawLineVal(graphCO2, ORANGE); drawLineVal(graphAQI, MAGENTA);
  }
}

void drawSyncButton(bool pressed) {
  if (currentTab != 2) return;
  uint16_t fillColor = pressed ? YELLOW : BLUE;
  display->fillRect(SYNC_BTN_X, SYNC_BTN_Y, SYNC_BTN_W, SYNC_BTN_H, fillColor);
  display->drawRect(SYNC_BTN_X, SYNC_BTN_Y, SYNC_BTN_W, SYNC_BTN_H, WHITE);
  display->setTextColor(BLACK, fillColor);
  display->setTextSize(3);
  display->setCursor(SYNC_BTN_X + 60, SYNC_BTN_Y + 20);
  display->print("SYNC");
}

void drawTab3() {
  display->fillRect(0, 41, 800, 439, BLACK);
  for (int i = 0; i < 32; i++) drawSingleButton(i);
  drawSyncButton(false);
}

void drawSingleButton(int i) {
  if (currentTab != 2) return;
  int col = i % 8, row = i / 8;
  int x = 20 + (col * 95), y = 60 + (row * 80);
  uint16_t color = relayState[i] ? GREEN : RED;
  display->fillRect(x, y, 80, 60, color);
  display->drawRect(x, y, 80, 60, WHITE);
  display->setTextColor(BLACK, color);
  display->setTextSize(2);
  display->setCursor((i + 1 < 10) ? x + 28 : x + 22, y + 22);
  display->printf("R%d", i + 1);
}

void toggleRelay(int index) {
  bool newState = !relayState[index];
  relayState[index] = newState;
  controlRelay(index + 1, newState);
  if (newState) relayBitmask |= (1UL << index);
  else relayBitmask &= ~(1UL << index);
  prefs.putUInt("relays", relayBitmask);
  if (currentTab == 2) drawSingleButton(index);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  prefs.begin("system", false);
  loadRelayStateFromPrefs();
  if (!display->begin()) while (1) delay(100);
  display->fillScreen(BLACK);
  tp.begin(); tp.setRotation(ROTATION_INVERTED);
  pinMode(GT911_INT_PIN, INPUT_PULLUP);
  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  delay(200);
  syncRelayBoardFromSavedState();
  requestRelayStatus();
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(2, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  if (esp_now_init() == ESP_OK) esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
  drawTabBar(); drawTab1();
}

void loop() {
  static int lastTouchedIndex = -1;
  static uint32_t debounceTimer = 0, lastPollTime = 0, lastTouchSeen = 0;
  if (newData) {
    recordGraphData();
    if (currentTab == 0) updateTab1Data();
    if (currentTab == 1) drawTab2();
    newData = false;
  }
  if (millis() - lastPollTime > 1000) { requestRelayStatus(); lastPollTime = millis(); }
  checkModbusFeedback();
  bool touchedNow = false;
  if ((millis() - lastTouchRead) >= 18) {
    lastTouchRead = millis();
    if (digitalRead(GT911_INT_PIN) == LOW) {
      tp.read();
      if (tp.isTouched && tp.touches > 0) { touchedNow = true; lastTouchSeen = millis(); }
    }
  }
  if (touchedNow) {
    int tx = tp.points[0].x, ty = tp.points[0].y;
    if (ty < 40) {
      int newTab = tx / 266; if (newTab > 2) newTab = 2;
      if (newTab != currentTab && (millis() - debounceTimer > 120)) {
        currentTab = newTab; drawTabBar();
        if (currentTab == 0) drawTab1(); else if (currentTab == 1) drawTab2(); else if (currentTab == 2) drawTab3();
        debounceTimer = millis();
      }
    } else if (currentTab == 2) {
      if (tx >= SYNC_BTN_X && tx < (SYNC_BTN_X + SYNC_BTN_W) && ty >= SYNC_BTN_Y && ty < (SYNC_BTN_Y + SYNC_BTN_H)) {
        if (millis() - debounceTimer > 120) { syncRelayBoardFromSavedState(); requestRelayStatus(); debounceTimer = millis(); }
      } else {
        int col = (tx - 20) / 95, row = (ty - 60) / 80;
        if (tx >= 20 && tx < (20 + 8 * 95) && ty >= 60 && ty < (60 + 4 * 80)) {
          int index = row * 8 + col;
          if (index >= 0 && index < 32 && index != lastTouchedIndex && (millis() - debounceTimer > 120)) {
            toggleRelay(index); lastTouchedIndex = index; debounceTimer = millis();
          }
        }
      }
    }
  } else if (millis() - lastTouchSeen > 70) { lastTouchedIndex = -1; }
  delay(4);
}