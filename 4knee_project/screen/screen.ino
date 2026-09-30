#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include "data.h"
#include "html.h"
#include "FS.h"
#include "LittleFS.h"

const char* ap_ssid = "KneeMonitor ";

const int k[4] = {1,2,3,4};

WebServer   server(80);
DNSServer   dnsServer;
Preferences prefs;
const byte  DNS_PORT = 53;

Recive_data       knee;
Setting_data      settings;
Graph_data        graph;
Advance_setting   adv;

volatile unsigned long lastRecvTime = 0;
volatile uint32_t      bootEpoch    = 0;

const unsigned long TIMEOUT_MS   = 3000;
unsigned long       lastLog      = 0;
const unsigned long LOG_INTERVAL = 1000;

unsigned long       lastRead      = 0;
const unsigned long READ_INTERVAL = 10;

const int           OVERSAMPLE    = 16;                      // reads per sample (more = less noise, slower)
const float         EMA_ALPHA     = 0.2f;                    // 0..1 (lower = smoother but reacts slower)
float               filt[4]       = {NAN, NAN, NAN, NAN};    // filtered raw ADC per channel

const int           HIST_NB      = 120;
const uint32_t      HIST_STEP[3] = {1UL, 5UL, 10UL};
float               hist[3][4][HIST_NB];
double              histSum[3][4];
uint32_t            histCnt[3]   = {0};
uint32_t            histSlot[3]  = {0};
uint8_t             histHead[3]  = {0};

uint8_t             keepMode     = 0;             
unsigned long       lastPurge    = 0;
const unsigned long PURGE_INTERVAL = 86400000UL;  
bool                purgedOnce   = false;

String jnum(double v) { return isnan(v) ? String("null") : String(v, 1); }

uint32_t nowEpoch() {
  if (bootEpoch == 0) return 0;
  return bootEpoch + (millis() / 1000);
}

uint32_t keepSeconds() { // week , month , year
  switch (keepMode) {
    case 1: return 604800UL;   
    case 2: return 2592000UL;  
    case 3: return 31536000UL;  
    default: return 0;
  }
}


void loadSettings() {
  prefs.begin("knee", true);

  settings.theme    = (Theme)   prefs.getUChar("theme", dark);
  settings.language = (Language)prefs.getUChar("lang",  english);
  settings.metric   = (Modify)  prefs.getUChar("metric", gram);
  settings.isauto   =           prefs.getBool ("auto",  true);

  adv.min           =           prefs.getDouble("min", 0.0);
  adv.max           =           prefs.getDouble("max", 100.0);

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

  prefs.putDouble("min",    adv.min);
  prefs.putDouble("max",    adv.max);

  prefs.end();
}

void savegraph() {
  prefs.begin("knee", false);
  prefs.putUChar("gtheme", graph.theme);
  prefs.putUChar("gtype",  graph.type);
  prefs.end();
}


// read one pin many times, drop highest + lowest (kills spikes), average the rest -> 0..4095
float readRaw(int pin) {
  uint32_t sum = 0; int lo = 4095, hi = 0;
  for (int i = 0; i < OVERSAMPLE; i++) {
    int v = analogRead(pin);
    sum += v;
    if (v < lo) lo = v;
    if (v > hi) hi = v;
  }
  return (sum - lo - hi) / (float)(OVERSAMPLE - 2);
}

// exponential moving average per channel
float smooth(int i, float raw) {
  if (isnan(filt[i])) filt[i] = raw;               // first reading: start right there, no slow ramp
  else                filt[i] += EMA_ALPHA * (raw - filt[i]);
  return filt[i];
}

void readSensors() {
  knee.knee1 = adv.min + (smooth(0, readRaw(k[0]))/4095.00) * (adv.max - adv.min);
  knee.knee2 = adv.min + (smooth(1, readRaw(k[1]))/4095.00) * (adv.max - adv.min);
  knee.knee3 = adv.min + (smooth(2, readRaw(k[2]))/4095.00) * (adv.max - adv.min);
  knee.knee4 = adv.min + (smooth(3, readRaw(k[3]))/4095.00) * (adv.max - adv.min);



  lastRecvTime = millis();

}

bool isConnected() {
  return (lastRecvTime != 0) && (millis() - lastRecvTime < TIMEOUT_MS);
}


void inithistory() {
  for (int s=0;s<3;s++)
    for (int i=0;i<4;i++)
      for (int b=0;b<HIST_NB;b++) hist[s][i][b] = NAN;
}

void addhistory(const Recive_data &x) {
  uint32_t sec = millis() / 1000;
  for (int s=0;s<3;s++) {
    uint32_t slot = sec / HIST_STEP[s];
    if (slot != histSlot[s]) {
      uint32_t gap = slot - histSlot[s];
      if (gap > (uint32_t)HIST_NB) gap = HIST_NB;
      for (uint32_t g=0;g<gap;g++) {
        for (int i=0;i<4;i++) hist[s][i][histHead[s]] = (g==0 && histCnt[s]) ? histSum[s][i]/histCnt[s] : NAN;
        histHead[s] = (histHead[s]+1) % HIST_NB;
      }
      histSlot[s] = slot;
      histCnt[s]  = 0;
      for (int i=0;i<4;i++) histSum[s][i] = 0;
    }
    histSum[s][0] += x.knee1; histSum[s][1] += x.knee2;
    histSum[s][2] += x.knee3; histSum[s][3] += x.knee4;
    histCnt[s]++;
  }
}


void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleData() {
  String j = "{";
  j += "\"k\":[" + String(knee.knee1,2) + "," + String(knee.knee2,2) + ","
                 + String(knee.knee3,2) + "," + String(knee.knee4,2) + "],";

  j += "\"a\":[" + String(adv.min,2)    + "," + String(adv.max,2)    + "],";

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

  double newMin = server.hasArg("min") ? server.arg("min").toDouble() : adv.min;
  double newMax = server.hasArg("max") ? server.arg("max").toDouble() : adv.max;
  if (newMin >= newMax) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"min must be less than max\"}");
    return;
  }
  adv.min = newMin;
  adv.max = newMax;

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
  int s = 0;
  if (server.hasArg("r")) {
    int r = server.arg("r").toInt();
    if (r==5) s = 1;
    else if (r==10) s = 2;
  }
  String j = "{\"k\":[";
  for (int i=0;i<4;i++){
    j += i ? ",[" : "[";
    for (int b=1;b<HIST_NB;b++) j += jnum(hist[s][i][(histHead[s]+b)%HIST_NB]) + ",";
    j += jnum(histCnt[s] ? histSum[s][i]/histCnt[s] : NAN) + "]";
  }
  j += "]}";
  server.send(200, "application/json", j);
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
  if (LittleFS.remove("/knee_data.csv")) Serial.println("CSV removed.");
  else                                   Serial.println("Error removing CSV.");
}


void purgeOlderThan(uint32_t cutoff) {
  if (nowEpoch() == 0) return;                 
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
  if (nowEpoch() == 0) return;                  
  if (!purgedOnce || (millis() - lastPurge >= PURGE_INTERVAL)) {
    purgeOlderThan(nowEpoch() - keepSeconds());
    lastPurge = millis();
    purgedOnce = true;
  }
}


void setup() {
  Serial.begin(115200);

  analogReadResolution(12);             
  analogSetAttenuation(ADC_11db);

  loadSettings();
  loadgraph();
  inithistory();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, NULL, 1);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  Serial.print("AP IP: ");         Serial.println(WiFi.softAPIP());

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }


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

  if (millis() - lastRead >= READ_INTERVAL) {
    readSensors();
    addhistory(knee);
    lastRead = millis();
  }

  if (isConnected() && (millis() - lastLog >= LOG_INTERVAL)) {
    writefile(knee);
    lastLog = millis();
  }

  maybePurge();
}