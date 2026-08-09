#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include "data.h"
#include "html.h"
#include "FS.h"
#include "LittleFS.h"

const char* ap_ssid = "KneeMonitor";

WebServer   server(80);
DNSServer   dnsServer;
Preferences prefs;
const byte  DNS_PORT = 53;

Recive_data  knee;
Setting_data settings;
Graph_data   graph;

volatile unsigned long lastRecvTime = 0;
volatile uint32_t      bootEpoch    = 0;

const unsigned long TIMEOUT_MS   = 3000;
unsigned long       lastLog      = 0;
const unsigned long LOG_INTERVAL = 1000;

uint8_t             keepMode     = 0;              // 0 off, 1 week, 2 month, 3 year
unsigned long       lastPurge    = 0;
const unsigned long PURGE_INTERVAL = 86400000UL;   // 24 ชม.
bool                purgedOnce   = false;


uint32_t nowEpoch() {
  if (bootEpoch == 0) return 0;
  return bootEpoch + (millis() / 1000);
}

uint32_t keepSeconds() {
  switch (keepMode) {
    case 1: return 604800UL;    // 7 วัน
    case 2: return 2592000UL;   // 30 วัน
    case 3: return 31536000UL;  // 365 วัน
    default: return 0;
  }
}


void loadSettings() {
  prefs.begin("knee", true);
  settings.theme    = (Theme)   prefs.getUChar("theme", dark);
  settings.language = (Language)prefs.getUChar("lang",  english);
  settings.metric   = (Modify)  prefs.getUChar("metric", gram);
  settings.isauto   =           prefs.getBool ("auto",  true);
  keepMode          =           prefs.getUChar("keep",  0);
  prefs.end();
}

void loadgraph() {
  prefs.begin("knee", true);
  graph.theme = (Theme)prefs.getUChar("gtheme", dark);
  graph.type  = (Type) prefs.getUChar("gtype",  line);
  prefs.end();
}

void saveSettings() {
  prefs.begin("knee", false);
  prefs.putUChar("theme",  settings.theme);
  prefs.putUChar("lang",   settings.language);
  prefs.putUChar("metric", settings.metric);
  prefs.putBool ("auto",   settings.isauto);
  prefs.putUChar("keep",   keepMode);
  prefs.end();
}

void savegraph() {
  prefs.begin("knee", false);
  prefs.putUChar("gtheme", graph.theme);
  prefs.putUChar("gtype",  graph.type);
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
  j += "\"connected\":"  + String(isConnected() ? "true" : "false")   + ",";
  j += "\"theme\":"      + String((int)settings.theme)                + ",";
  j += "\"lang\":"       + String((int)settings.language)             + ",";
  j += "\"metric\":"     + String((int)settings.metric)               + ",";
  j += "\"auto\":"       + String(settings.isauto ? "true" : "false") + ",";
  j += "\"graphtheme\":" + String((int)graph.theme)                   + ",";
  j += "\"graphtype\":"  + String((int)graph.type)                    + ",";
  j += "\"keep\":"       + String((int)keepMode);
  j += "}";
  server.send(200, "application/json", j);
}

void handleSet() {
  if (server.hasArg("theme"))      settings.theme    = (Theme)   server.arg("theme").toInt();
  if (server.hasArg("lang"))       settings.language = (Language)server.arg("lang").toInt();
  if (server.hasArg("metric"))     settings.metric   = (Modify)  server.arg("metric").toInt();
  if (server.hasArg("auto"))       settings.isauto   = (server.arg("auto").toInt() != 0);
  if (server.hasArg("graphtheme")) graph.theme       = (Theme)   server.arg("graphtheme").toInt();
  if (server.hasArg("graphtype"))  graph.type        = (Type)    server.arg("graphtype").toInt();
  if (server.hasArg("keep")) {                        // validate 0..3
    int v = server.arg("keep").toInt();
    if (v < 0) v = 0; if (v > 3) v = 3;
    keepMode = (uint8_t)v;
  }
  saveSettings();
  savegraph();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleTime() {
  if (server.hasArg("t")) {
    uint32_t clientEpoch = strtoul(server.arg("t").c_str(), NULL, 10);
    if (clientEpoch > 1700000000UL) {
      bootEpoch = clientEpoch - (millis() / 1000);
    }
  }
  server.send(200, "text/plain", "ok");
}

void handleDownload() {
  File file = LittleFS.open("/knee_data.csv", "r");
  if (!file) { server.send(404, "text/plain", "No data file"); return; }
  server.sendHeader("Content-Disposition", "attachment; filename=\"knee_data.csv\"");
  server.streamFile(file, "text/csv");
  file.close();
}

void handleHistory() {
  uint32_t now = nowEpoch();
  if (now == 0) { server.send(200,"application/json","{\"err\":\"notime\"}"); return; }

  uint32_t rangeSec = 604800UL;                 // week default
  if (server.hasArg("r")) {
    char c = server.arg("r").charAt(0);
    if (c=='m') rangeSec = 2592000UL;
    else if (c=='y') rangeSec = 31536000UL;
  }

  static const int NB = 120;
  static float    sum[4][NB];
  static uint16_t cnt[NB];
  for (int b=0;b<NB;b++){ cnt[b]=0; for(int i=0;i<4;i++) sum[i][b]=0; }

  uint32_t start     = (now > rangeSec) ? now - rangeSec : 0;
  uint32_t bucketSec = rangeSec / NB;

  File f = LittleFS.open("/knee_data.csv","r");
  if (!f) { server.send(404,"application/json","{\"err\":\"nofile\"}"); return; }

  char line[96];
  f.readBytesUntil('\n', line, sizeof(line)-1);   // ข้าม header
  while (f.available()) {
    int n = f.readBytesUntil('\n', line, sizeof(line)-1);
    line[n] = 0;
    if (n < 5) continue;
    unsigned long ts; float a,b,c,d;
    if (sscanf(line,"%lu,%f,%f,%f,%f",&ts,&a,&b,&c,&d)==5) {
      if (ts >= start && ts <= now) {
        int bi = (ts - start)/bucketSec;
        if (bi < 0) bi = 0; if (bi >= NB) bi = NB-1;
        sum[0][bi]+=a; sum[1][bi]+=b; sum[2][bi]+=c; sum[3][bi]+=d; cnt[bi]++;
      }
    }
  }
  f.close();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200,"application/json","");
  char chunk[64];
  snprintf(chunk,sizeof(chunk),"{\"start\":%lu,\"dt\":%lu,\"k\":[",(unsigned long)start,(unsigned long)bucketSec);
  server.sendContent(chunk);
  for (int i=0;i<4;i++){
    server.sendContent(i ? ",[" : "[");
    for (int b=0;b<NB;b++){
      float v = cnt[b] ? sum[i][b]/cnt[b] : -1;
      snprintf(chunk,sizeof(chunk),"%s%.1f", b?",":"", v);
      server.sendContent(chunk);
    }
    server.sendContent("]");
  }
  server.sendContent("]}");
  server.sendContent("");
}

void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}


void writefile(const Recive_data &x) {
  File file = LittleFS.open("/knee_data.csv", "a");
  if (!file) { Serial.println("open fail"); return; }
  uint32_t ts = nowEpoch();
  if (ts == 0) ts = millis() / 1000;
  file.print(ts);         file.print(",");
  file.print(x.knee1, 2); file.print(",");
  file.print(x.knee2, 2); file.print(",");
  file.print(x.knee3, 2); file.print(",");
  file.println(x.knee4, 2);
  file.close();
}

void initfile() {
  if (!LittleFS.exists("/knee_data.csv")) {
    File file = LittleFS.open("/knee_data.csv", "w");
    if (!file) { Serial.println("Error creating file!"); return; }
    file.println("time,knee1,knee2,knee3,knee4");
    file.close();
    Serial.println("CSV created with header.");
  }
}

void deletefile() {
  File file = LittleFS.open("/knee_data.csv", "w");
  if (file) { file.close(); Serial.println("CSV cleared."); }
  else      { Serial.println("Error clearing CSV file."); }
}

// อ่านทั้งไฟล์ กรองเฉพาะแถวที่ใหม่กว่า cutoff เขียนไฟล์ใหม่ (stream, RAM คงที่)
void purgeOlderThan(uint32_t cutoff) {
  if (nowEpoch() == 0) return;                  // ยังไม่ sync เวลา อย่าลบ
  File in = LittleFS.open("/knee_data.csv", "r");
  if (!in) return;
  File out = LittleFS.open("/tmp.csv", "w");
  if (!out) { in.close(); return; }

  char line[96];
  int n = in.readBytesUntil('\n', line, sizeof(line)-1);   // header
  line[n] = 0; out.print(line); out.print("\n");

  while (in.available()) {
    n = in.readBytesUntil('\n', line, sizeof(line)-1);
    line[n] = 0;
    if (n < 5) continue;
    uint32_t ts = strtoul(line, NULL, 10);
    if (ts >= cutoff) { out.print(line); out.print("\n"); }
  }
  in.close();
  out.close();
  LittleFS.remove("/knee_data.csv");
  LittleFS.rename("/tmp.csv", "/knee_data.csv");
  Serial.println("Purged old rows.");
}

void maybePurge() {
  if (keepMode == 0) return;
  if (nowEpoch() == 0) return;                  // ต้อง sync เวลาก่อน
  if (!purgedOnce || (millis() - lastPurge >= PURGE_INTERVAL)) {
    purgeOlderThan(nowEpoch() - keepSeconds());
    lastPurge = millis();
    purgedOnce = true;
  }
}


void setup() {
  Serial.begin(115200);

  loadSettings();
  loadgraph();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, NULL, 1);
  Serial.print("AP IP: ");         Serial.println(WiFi.softAPIP());
  Serial.print("Receiver MAC: ");  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // กู้คืนถ้า purge ค้างกลางคัน (ไฟดับตอน rename)
  if (LittleFS.exists("/tmp.csv")) {
    if (!LittleFS.exists("/knee_data.csv")) LittleFS.rename("/tmp.csv", "/knee_data.csv");
    else                                    LittleFS.remove("/tmp.csv");
  }
  initfile();

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/",         handleRoot);
  server.on("/data",     handleData);
  server.on("/set",      handleSet);
  server.on("/time",     handleTime);
  server.on("/history",  handleHistory);
  server.on("/download", handleDownload);
  server.on("/clear",    [](){ deletefile(); initfile(); server.send(200,"text/plain","cleared"); });
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (isConnected() && (millis() - lastLog >= LOG_INTERVAL)) {
    writefile(knee);
    lastLog = millis();
  }
  maybePurge();
}