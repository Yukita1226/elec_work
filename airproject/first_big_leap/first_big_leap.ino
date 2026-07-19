float o3_ppm = 0;
float tvoc = 0;
float eco2 = 0;
int aqi = 0;
int pmx =0;
#include "lgfx_user_setup.h"
#include "data.h"
#include "background.h"
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

#define CBLUE   0x1BDC
#define CGREEN  0x2D6A
#define CPURPLE 0x7AB8
#define LTGRAY  0xC618
#define DBLUE   0x11EB

#define SYNC_BTN_X 300
#define SYNC_BTN_Y 390
#define SYNC_BTN_W 200
#define SYNC_BTN_H 60

#define ON_BTN_X  565
#define ON_BTN_Y  380
#define OFF_BTN_X 675
#define OFF_BTN_Y 380
#define PWR_BTN_W 100
#define PWR_BTN_H 70

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
void drawPowerButtons(int pressed);
void setAllRelays(bool state);

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

void setAllRelays(bool state) {
  for (int i = 0; i < 32; i++) {
    relayState[i] = state;
    controlRelay(i + 1, state);
    delay(20);
  }
  relayBitmask = state ? 0xFFFFFFFF : 0;
  prefs.putUInt("relays", relayBitmask);
}

void recordGraphData() {
  graphPM1[graphHead]  = (float)incomingData.pm1_0;
  graphPM25[graphHead] = (float)incomingData.pm2_5;
  graphPM10[graphHead] = (float)incomingData.pm10;
  graphO3[graphHead]   = o3_ppm;
  graphVOC[graphHead]  = tvoc;
  graphCO2[graphHead]  = eco2;
  graphAQI[graphHead]  = aqi;
  
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

LGFX_Sprite bgSprite(&lcd);
bool bgInit = false;

void makeBG() {
  bgSprite.setPsram(true);
  bgSprite.setColorDepth(16);
  if (!bgSprite.createSprite(800, 439)) return;
  bgSprite.drawJpg(bg_jpg, bg_jpg_len, 0, 0);
  bgInit = true;
}

void restoreBG(int x, int y, int w, int h) {
  if (!bgInit) { display->fillRect(x, y, w, h, WHITE); return; }
  static uint16_t line[800];
  for (int j = 0; j < h; j++) {
    bgSprite.readRect(x, y - 41 + j, w, 1, line);
    display->pushImage(x, y + j, w, 1, line);
  }
}

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
  display->setTextSize(3);
  display->setCursor(100, 12); display->print("DATA");
  display->setCursor(360, 12); display->print("CHART");
  display->setCursor(630, 12); display->print("CTRL");
  display->drawFastVLine(266, 0, 40, BLACK);
  display->drawFastVLine(532, 0, 40, BLACK);
  display->drawFastHLine(0, 40, 800, WHITE);
}


void drawTab1() {
  if (!bgInit) makeBG();
  if (bgInit) bgSprite.pushSprite(display, 0, 41);
  else display->drawJpg(bg_jpg, bg_jpg_len, 0, 41);

  display->fillRoundRect(20, 160, 240, 32, 6, CBLUE);
  display->drawRoundRect(20, 160, 240, 160, 6, LTGRAY);
  display->setTextColor(WHITE);
  display->setTextSize(2);
  display->setCursor(68, 169);
  display->print("PARTICULATES");

  display->fillRoundRect(280, 160, 240, 32, 6, CGREEN);
  display->drawRoundRect(280, 160, 240, 160, 6, LTGRAY);
  display->setTextColor(WHITE);
  display->setCursor(370, 169);
  display->print("GASES");

  display->fillRoundRect(540, 160, 240, 32, 6, CPURPLE);
  display->drawRoundRect(540, 160, 240, 160, 6, LTGRAY);
  display->setTextColor(WHITE);
  display->setCursor(636, 169);
  display->print("ROLE");

  display->setTextColor(BLACK);
  display->setTextSize(2);
  display->setCursor(34, 211);  display->print("PM1");
  display->setCursor(34, 251);  display->print("PM2.5");
  display->setCursor(34, 291);  display->print("PM10");
  display->setCursor(196, 211); display->print("ug/m3");
  display->setCursor(196, 251); display->print("ug/m3");
  display->setCursor(196, 291); display->print("ug/m3");

  display->setCursor(294, 211); display->print("CO2");
  display->setCursor(294, 251); display->print("VOC");
  display->setCursor(462, 211); display->print("ppm");
  display->setCursor(462, 251); display->print("ppm");

  display->setCursor(554, 211); display->print("O3");
  display->setCursor(554, 251); display->print("CO2");
  display->setCursor(554, 291); display->print("VOC");
  display->setCursor(730, 211); display->print("ppm");
  display->setCursor(730, 251); display->print("ppm");
  display->setCursor(730, 291); display->print("ppm");

  display->drawRoundRect(20, 330, 520, 120, 6, LTGRAY);
  display->setTextColor(DBLUE);
  display->setTextSize(2);
  display->setCursor(226, 338);
  display->print("AQI INDEX");

  display->fillRect(60, 404, 88, 22, GREEN);
  display->fillRect(148, 404, 88, 22, CYAN);
  display->fillRect(236, 404, 88, 22, YELLOW);
  display->fillRect(324, 404, 88, 22, ORANGE);
  display->fillRect(412, 404, 88, 22, RED);
  display->drawRect(60, 404, 440, 22, BLACK);

  display->setTextColor(DBLUE);
  display->setTextSize(2);
  display->setCursor(630, 338);
  display->print("CONTROL");
  drawPowerButtons(0);

  updateTab1Data();
}


void drawPowerButtons(int pressed) {
  uint16_t onColor  = (pressed == 1) ? YELLOW : CGREEN;
  uint16_t offColor = (pressed == 2) ? YELLOW : RED;
  display->fillRoundRect(ON_BTN_X, ON_BTN_Y, PWR_BTN_W, PWR_BTN_H, 8, onColor);
  display->drawRoundRect(ON_BTN_X, ON_BTN_Y, PWR_BTN_W, PWR_BTN_H, 8, DKGRAY);
  display->setTextColor(WHITE);
  display->setTextSize(3);
  display->setCursor(ON_BTN_X + 32, ON_BTN_Y + 24);
  display->print("ON");

  display->fillRoundRect(OFF_BTN_X, OFF_BTN_Y, PWR_BTN_W, PWR_BTN_H, 8, offColor);
  display->drawRoundRect(OFF_BTN_X, OFF_BTN_Y, PWR_BTN_W, PWR_BTN_H, 8, DKGRAY);
  display->setTextColor(WHITE);
  display->setCursor(OFF_BTN_X + 23, OFF_BTN_Y + 24);
  display->print("OFF");
}


void updateTab1Data() {
  if (currentTab != 0) return;

  restoreBG(108, 203, 86, 26);
  restoreBG(108, 243, 86, 26);
  restoreBG(108, 283, 86, 26);
  restoreBG(358, 203, 100, 26);
  restoreBG(358, 243, 100, 26);
  restoreBG(618, 203, 100, 26);
  restoreBG(618, 243, 100, 26);
  restoreBG(618, 283, 100, 26);

  display->setTextSize(3);
  display->setTextColor(CBLUE);
  display->setCursor(110, 205); display->printf("%-4d", incomingData.pm1_0);
  display->setCursor(110, 245); display->printf("%-4d", incomingData.pm2_5);
  display->setCursor(110, 285); display->printf("%-4d", incomingData.pm10);

  display->setTextColor(CGREEN);
  display->setCursor(360, 205); display->printf("%-5.2f", eco2);
  display->setCursor(360, 245); display->printf("%-5.2f", tvoc);

  display->setTextColor(CPURPLE);
  display->setCursor(620, 205); display->printf("%-5.2f", o3_ppm);
  display->setCursor(620, 245); display->printf("%-5.2f", eco2);
  display->setCursor(620, 285); display->printf("%-5.2f", tvoc);

  if (incomingData.pm2_5 >= 91|| incomingData.pm10 >= 181)
  {
    incomingData.aqi = 5;
  }
  else if (incomingData.pm2_5 >= 51|| incomingData.pm10 >= 121)
  {
    incomingData.aqi = 4;
  }
  else if (incomingData.pm2_5 >= 38|| incomingData.pm10 >= 81)
  {
    incomingData.aqi = 3;
  }
  else if (incomingData.pm2_5 >= 26|| incomingData.pm10 >= 51)
  {
    incomingData.aqi = 2;
  }
  else 
  {
    incomingData.aqi = 1;
  }

  restoreBG(186, 360, 230, 30);
  display->setTextSize(3);
  display->setCursor(190, 364);

  if (incomingData.aqi == 1)
  {
    display->setTextColor(GREEN);  
    display->print(" VERY GOOD");
  } 
  else if (incomingData.aqi == 2)
  {
    display->setTextColor(CYAN);  
    display->print("    GOOD  ");
  }
  else if (incomingData.aqi == 3)
  {
    display->setTextColor(YELLOW);  
    display->print("     OK   ");
  }
  else if (incomingData.aqi == 4)
  {
    display->setTextColor(ORANGE);  
    display->print("    POOR  ");
  }
  else if (incomingData.aqi == 5)
  {
    display->setTextColor(RED);  
    display->print("     BAD   ");
  }
  else display->print("  WAIT  ");

  restoreBG(56, 386, 448, 16);
  int cx = 60 + (incomingData.aqi - 1) * 88 + 44;
  if (incomingData.aqi >= 1 && incomingData.aqi <= 5) {
    display->fillTriangle(cx - 8, 387, cx + 8, 387, cx, 401, BLACK);
  }
}

void drawTab2() {
  if (!bgInit) makeBG();
  if (bgInit) bgSprite.pushSprite(display, 0, 41);
  else display->drawJpg(bg_jpg, bg_jpg_len, 0, 41);
  
  display->drawLine(60, 430, 560, 430, BLACK);
  display->drawLine(60, 150, 60, 430, BLACK);

  display->fillRect(570, 150, 210, 160, WHITE);
  display->drawRect(570, 150, 210, 160, BLACK);
  display->setTextSize(2);
  display->setTextColor(BLACK, WHITE);   display->setCursor(580, 160); display->printf("PM1.0: %-4d", (int)incomingData.pm1_0);
  display->setTextColor(RED, WHITE);     display->setCursor(580, 180); display->printf("PM2.5: %-4d", (int)incomingData.pm2_5);
  display->setTextColor(GREEN, WHITE);   display->setCursor(580, 200); display->printf("PM10 : %-4d", (int)incomingData.pm10);

  display->setTextColor(CYAN, WHITE);    display->setCursor(580, 220); display->printf("O3   : %-5.2f", o3_ppm);
  display->setTextColor(YELLOW, WHITE);  display->setCursor(580, 240); display->printf("VOC  : %-5.2f", tvoc);
  display->setTextColor(ORANGE, WHITE);  display->setCursor(580, 260); display->printf("CO2  : %-5.2f", eco2);
  display->setTextColor(MAGENTA, WHITE); display->setCursor(580, 280); display->printf("AQI  : %-5d", aqi);
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
  display->setTextColor(BLACK, WHITE);
  display->setCursor(5, 150); display->printf("%4d", (int)max_v);
  display->setCursor(5, 285); display->printf("%4d", (int)((max_v + min_v) / 2));
  display->setCursor(5, 414); display->printf("%4d", (int)min_v);

  int stepX = 500 / MAX_GRAPH_POINTS;
  for (int i = 0; i < graphCount - 1; i++) {
    int idx1 = (graphHead - graphCount + i + MAX_GRAPH_POINTS) % MAX_GRAPH_POINTS;
    int idx2 = (idx1 + 1) % MAX_GRAPH_POINTS;
    int x1 = 60 + i * stepX, x2 = 60 + (i + 1) * stepX;
    auto drawLineVal = [&](float g[], uint16_t c) {
      int y1 = 430 - (int)((g[idx1] - min_v) / range * 280);
      int y2 = 430 - (int)((g[idx2] - min_v) / range * 280);
      display->drawWideLine(x1, y1, x2, y2, 1.7f, c);
    };
    drawLineVal(graphPM1, BLACK); drawLineVal(graphPM25, RED); drawLineVal(graphPM10, GREEN);
    drawLineVal(graphO3, CYAN); drawLineVal(graphVOC, YELLOW); drawLineVal(graphCO2, ORANGE); drawLineVal(graphAQI, MAGENTA);
  }
}

void drawSyncButton(bool pressed) {
  if (currentTab != 2) return;
  uint16_t fillColor = pressed ? YELLOW : BLUE;
  display->fillRoundRect(SYNC_BTN_X, SYNC_BTN_Y, SYNC_BTN_W, SYNC_BTN_H,6, fillColor);
  display->drawRoundRect(SYNC_BTN_X, SYNC_BTN_Y, SYNC_BTN_W, SYNC_BTN_H,6, WHITE);
  display->setTextColor(BLACK, fillColor);
  display->setTextSize(3);
  display->setCursor(SYNC_BTN_X + 60, SYNC_BTN_Y + 20);
  display->print("SYNC");
}

void drawTab3() {
  display->fillRect(0, 41, 800, 439, WHITE);
  for (int i = 0; i < 32; i++) drawSingleButton(i);
  drawSyncButton(false);
}

void drawSingleButton(int i) {
  if (currentTab != 2) return;
  int col = i % 8, row = i / 8;
  int x = 20 + (col * 95), y = 60 + (row * 80);
  uint16_t color = relayState[i] ? GREEN : RED;
  display->fillRoundRect(x, y, 80, 60, 6, color);
  display->drawRoundRect(x, y, 80, 60, 6, LTGRAY);
  display->setTextColor(BLACK);
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

pmx = incomingData.pm2_5;
datax();
//  o3_ppm = map(incomingData.pm2_5,0,500,0.0,0.5);
//  tvoc = map(incomingData.pm2_5,0,500,0,0.5);
//  eco2 = map(incomingData.pm2_5,0,500,0,0.5);
//  aqi = map((int)incomingData.pm2_5,0,500,0,0.5);


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
    } else if (currentTab == 0) {
      if (ty >= ON_BTN_Y && ty < (ON_BTN_Y + PWR_BTN_H) && (millis() - debounceTimer > 300)) {
        if (tx >= ON_BTN_X && tx < (ON_BTN_X + PWR_BTN_W)) {
          drawPowerButtons(1);
          setAllRelays(true);
          drawPowerButtons(0);
          if (currentTab == 2) {} 
          debounceTimer = millis();
        } else if (tx >= OFF_BTN_X && tx < (OFF_BTN_X + PWR_BTN_W)) {
          drawPowerButtons(2);
          setAllRelays(false);
          drawPowerButtons(0);
          debounceTimer = millis();
        }
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