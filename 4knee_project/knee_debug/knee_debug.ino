/*
 ============================================================================
   KNEE MONITOR  -  SCREEN (RECEIVER) DEBUG FIRMWARE
 ============================================================================
   Upload this INSTEAD of screen.ino to find out why data stops.
   It behaves like the real screen (same Wi-Fi name, channel 1, ESP-NOW,
   captive portal, 500 ms polling page) but logs EVERYTHING.

   HOW TO USE
    1. Arduino IDE -> open knee_debug/knee_debug.ino -> upload to the SCREEN ESP32.
    2. Open Serial Monitor at 115200 baud and leave it open.
    3. Turn on the sensor (sender) ESP32.
    4. Connect your phone to the "KneeMonitor" Wi-Fi. The debug page opens
       by itself (or go to http://192.168.4.1).
    5. Wait until the data stops, then wait 20 more seconds.
    6. Copy ALL the Serial Monitor text + take a screenshot of the phone page.

   HOW TO READ THE LOG
    ">>"   = good thing happened (link up, phone connected ...)
    "!!"   = problem
    "HINT" = what the problem most likely means
    A full REPORT block is printed every 2 seconds.

   SERIAL COMMANDS (type in Serial Monitor, press Enter)
    r = full report now      e = print all stored events
    v = verbose on/off (print every packet)
    s = re-scan knee_data.csv    h = help

   SAFE: does NOT delete or change your knee_data.csv or saved settings.
         It only writes a small test file /diag_test.csv.
   Needs Arduino-ESP32 core 3.x (same as screen.ino).
 ============================================================================
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_idf_version.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <FS.h>
#include <LittleFS.h>
#include <stdarg.h>
#include <time.h>
#include <cmath>

// ============================== CONFIG =====================================
#define AP_SSID            "KneeMonitor"
#define AP_CHANNEL         1
#define CAPTIVE_PORTAL     1     // 1 = same as real firmware. 0 = no DNS hijack (A/B test)
#define DISABLE_WIFI_SLEEP 0     // 0 = same as real firmware. 1 = WiFi.setSleep(false) (A/B test)
#define TEST_CSV_WRITE     1     // write 1 row/s to /diag_test.csv while sensor linked + time it
#define SCAN_REAL_CSV      1     // at boot, READ /knee_data.csv (no changes) and time it
#define REPORT_MS          2000  // full report interval
#define SENSOR_TIMEOUT_MS  3000  // same as real firmware
#define STALL_WARN_MS      100   // log anything that blocks longer than this
#define GAP_WARN_MS        500   // log ESP-NOW gaps longer than this
#define HTTP_SILENT_MS     3000  // warn if a connected phone stops polling this long

struct Recive_data { float knee1, knee2, knee3, knee4; };   // MUST match the sender
static const int EXPECTED_LEN = sizeof(Recive_data);

// ---- all types here, ABOVE every function (Arduino IDE auto-prototype rule) ----
#define EV_N   80
#define EV_LEN 120
struct Ev { uint32_t id; uint32_t ms; char msg[EV_LEN]; };
struct RxState {
  uint32_t total, ok, bad, nan;
  int      badLen;
  unsigned long firstMs, lastMs, lastGoodMs;
  uint32_t maxGapWin;
  int      rssi, rssiMinWin, rssiMaxWin;
  uint8_t  src[6], dst[6];
  Recive_data v;
};
struct SenderInfo { uint8_t mac[6]; uint32_t n; };
struct Sect { const char* name; uint32_t maxWin, maxEver; };

WebServer   server(80);
DNSServer   dnsServer;
Preferences prefs;
bool        fsOk     = false;
uint8_t     keepMode = 0;
uint8_t     staMac[6] = {0}, apMac[6] = {0};
uint32_t    bootEpoch = 0;
bool        verbose   = false;

// ============================ HELPERS ======================================
void macStr(const uint8_t* m, char* out) {
  if (!m) { strcpy(out, "??"); return; }
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

void addf(String& o, const char* fmt, ...) {
  char b[320];
  va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof(b), fmt, ap); va_end(ap);
  o += b;
}

const char* resetReasonStr(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "POWER ON (normal)";
    case ESP_RST_EXT:       return "EXTERNAL PIN";
    case ESP_RST_SW:        return "SOFTWARE restart";
    case ESP_RST_PANIC:     return "!! CRASH (panic / exception)";
    case ESP_RST_INT_WDT:   return "!! CRASH (interrupt watchdog)";
    case ESP_RST_TASK_WDT:  return "!! CRASH (task watchdog - something blocked too long)";
    case ESP_RST_WDT:       return "!! CRASH (other watchdog)";
    case ESP_RST_DEEPSLEEP: return "wake from deep sleep";
    case ESP_RST_BROWNOUT:  return "!! BROWNOUT (power supply voltage dropped - weak USB/battery)";
    case ESP_RST_SDIO:      return "SDIO";
    default:                return "UNKNOWN";
  }
}

const char* dstKind(const uint8_t* d) {
  static const uint8_t bc[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (!memcmp(d, staMac, 6)) return "this ESP STA MAC, unicast";
  if (!memcmp(d, apMac, 6))  return "this ESP AP MAC, unicast";
  if (!memcmp(d, bc, 6))     return "BROADCAST";
  return "!! some OTHER MAC";
}

// ============================ EVENT LOG ====================================
// Ring buffer, safe to call from any task (Wi-Fi callbacks too).
// loop() prints new events to Serial, /data sends them to the phone page.
Ev evRing[EV_N];
volatile uint32_t evCount = 0;
uint32_t evPrinted = 0;
portMUX_TYPE evMux = portMUX_INITIALIZER_UNLOCKED;

void logEvent(const char* fmt, ...) {
  char buf[EV_LEN];
  va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
  uint32_t ms = millis();
  portENTER_CRITICAL(&evMux);
  uint32_t id = evCount;
  Ev& e = evRing[id % EV_N];
  e.id = id; e.ms = ms; memcpy(e.msg, buf, EV_LEN);
  evCount = id + 1;
  portEXIT_CRITICAL(&evMux);
}

bool getEvent(uint32_t id, Ev& out) {
  bool ok = false;
  portENTER_CRITICAL(&evMux);
  if (id < evCount && evCount - id <= EV_N) { out = evRing[id % EV_N]; ok = true; }
  portEXIT_CRITICAL(&evMux);
  return ok;
}

void drainEvents() {
  while (evPrinted < evCount) {
    if (evCount - evPrinted > EV_N) {
      Serial.printf("... %lu events lost (log overflow)\n", (unsigned long)(evCount - evPrinted - EV_N));
      evPrinted = evCount - EV_N;
    }
    Ev e;
    if (getEvent(evPrinted, e)) Serial.printf("[%9.3f] %s\n", e.ms / 1000.0, e.msg);
    evPrinted++;
  }
}

// ============================ ESP-NOW RX ===================================
RxState rx = {};
portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;

SenderInfo senders[4];
int nSenders = 0;

// Runs in the Wi-Fi task (NOT in loop). A busy loop() cannot block this.
void onDataRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  unsigned long now = millis();
  int rssi = (info && info->rx_ctrl) ? info->rx_ctrl->rssi : 0;
  uint32_t gap = 0, badCount = 0, nanCount = 0;
  bool first = false, badNow = false, nanNow = false, newSender = false;

  portENTER_CRITICAL(&rxMux);
  rx.total++;
  if (rx.lastMs) { gap = now - rx.lastMs; if (gap > rx.maxGapWin) rx.maxGapWin = gap; }
  else { first = true; rx.firstMs = now; }
  rx.lastMs = now;
  rx.rssi = rssi;
  if (rssi < rx.rssiMinWin) rx.rssiMinWin = rssi;
  if (rssi > rx.rssiMaxWin) rx.rssiMaxWin = rssi;
  if (info->src_addr) memcpy(rx.src, info->src_addr, 6);
  if (info->des_addr) memcpy(rx.dst, info->des_addr, 6);
  int idx = -1;
  for (int i = 0; i < nSenders; i++) if (!memcmp(senders[i].mac, info->src_addr, 6)) { senders[i].n++; idx = i; break; }
  if (idx < 0 && nSenders < 4) { memcpy(senders[nSenders].mac, info->src_addr, 6); senders[nSenders].n = 1; nSenders++; newSender = true; }
  if (len == EXPECTED_LEN) {
    memcpy(&rx.v, data, len);
    rx.ok++; rx.lastGoodMs = now;
    if (!std::isfinite(rx.v.knee1) || !std::isfinite(rx.v.knee2) ||
        !std::isfinite(rx.v.knee3) || !std::isfinite(rx.v.knee4)) { rx.nan++; nanNow = true; nanCount = rx.nan; }
  } else {
    rx.bad++; rx.badLen = len; badNow = true; badCount = rx.bad;
  }
  portEXIT_CRITICAL(&rxMux);

  char m[18]; macStr(info->src_addr, m);
  if (first)          logEvent(">> FIRST ESP-NOW packet: from %s, len=%d, rssi=%d dBm", m, len, rssi);
  else if (gap >= GAP_WARN_MS) logEvent("ESP-NOW packet after a GAP of %lu ms (from %s, rssi %d)", (unsigned long)gap, m, rssi);
  if (newSender && !first) logEvent("!! a SECOND device is sending ESP-NOW: %s", m);
  if (badNow && (badCount <= 3 || badCount % 100 == 0))
    logEvent("!! WRONG SIZE packet #%lu: len=%d but screen expects %d -> packet IGNORED", (unsigned long)badCount, len, EXPECTED_LEN);
  if (nanNow && (nanCount <= 3 || nanCount % 100 == 0))
    logEvent("!! packet contains NaN/inf values (#%lu) - sensor read error on sender?", (unsigned long)nanCount);
}

RxState snapRx(bool resetWindow) {
  RxState s;
  portENTER_CRITICAL(&rxMux);
  s = rx;
  if (resetWindow) { rx.maxGapWin = 0; rx.rssiMinWin = 127; rx.rssiMaxWin = -127; }
  portEXIT_CRITICAL(&rxMux);
  return s;
}

// ============================ WI-FI EVENTS =================================
volatile unsigned long lastPhoneConnMs = 0, lastPhoneDiscMs = 0;
volatile uint32_t probeReq = 0, phoneConnects = 0, phoneDisconnects = 0;

void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  char m[18];
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START: logEvent("WIFI: access point started"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:  logEvent("!! WIFI: access point STOPPED"); break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      macStr(info.wifi_ap_staconnected.mac, m);
      lastPhoneConnMs = millis(); phoneConnects++;
      logEvent(">> PHONE CONNECTED to Wi-Fi: %s (aid %d)", m, (int)info.wifi_ap_staconnected.aid);
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      macStr(info.wifi_ap_stadisconnected.mac, m);
      lastPhoneDiscMs = millis(); phoneDisconnects++;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
      logEvent("!! PHONE DISCONNECTED from Wi-Fi: %s (aid %d, reason %d)", m,
               (int)info.wifi_ap_stadisconnected.aid, (int)info.wifi_ap_stadisconnected.reason);
#else
      logEvent("!! PHONE DISCONNECTED from Wi-Fi: %s (aid %d)", m, (int)info.wifi_ap_stadisconnected.aid);
#endif
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      logEvent("PHONE got IP %s", IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED: probeReq++; break;
    case ARDUINO_EVENT_WIFI_READY: break;
    case ARDUINO_EVENT_WIFI_STA_START: logEvent("WIFI: STA interface started (idle, not connecting)"); break;
    case ARDUINO_EVENT_WIFI_STA_STOP:  logEvent("WIFI: STA interface stopped"); break;
    default: logEvent("WIFI event #%d", (int)event); break;
  }
}

// ============================ LOOP TIMING ==================================
Sect secDns  = {"dns", 0, 0};
Sect secHttp = {"http", 0, 0};
Sect secCsv  = {"csv-write", 0, 0};
bool hSlowFlag = false;
uint32_t loopsWin = 0;

void sectDone(Sect& S, uint32_t d) {
  if (d > S.maxWin)  S.maxWin = d;
  if (d > S.maxEver) S.maxEver = d;
  if (d >= STALL_WARN_MS) {
    if (&S == &secHttp && !hSlowFlag)
      logEvent("!! LOOP BLOCKED %lu ms in web server (not a handler: waiting on a slow/idle phone connection)", (unsigned long)d);
    else if (&S != &secHttp)
      logEvent("!! LOOP BLOCKED %lu ms in %s", (unsigned long)d, S.name);
  }
}
#define TIMED(S, stmt) do { unsigned long _t0 = millis(); stmt; sectDone(S, millis() - _t0); } while (0)

// ============================ HTTP =========================================
enum { EP_ROOT, EP_DATA, EP_TIME, EP_DIAG, EP_CAPTIVE, EP_N };
const char* EP_NAME[EP_N] = {"/", "/data", "/time", "/diag", "captive/404"};
uint32_t epWin[EP_N] = {0}, epTotal[EP_N] = {0};
uint32_t httpSlowWin = 0; const char* httpSlowName = "-";
unsigned long lastHttpMs = 0, hT0 = 0;
IPAddress lastHttpIP, curIP;
uint32_t captiveLoggedWin = 0;
bool httpSilentWarned = false;

void hBegin() { hT0 = millis(); curIP = server.client().remoteIP(); }
void hEnd(int ep) {
  uint32_t d = millis() - hT0;
  epWin[ep]++; epTotal[ep]++;
  if (d > httpSlowWin) { httpSlowWin = d; httpSlowName = EP_NAME[ep]; }
  unsigned long now = millis();
  if (ep != EP_CAPTIVE) {
    if (lastHttpMs == 0)                          logEvent(">> FIRST page request from phone %s (%s)", curIP.toString().c_str(), EP_NAME[ep]);
    else if (now - lastHttpMs > HTTP_SILENT_MS)   logEvent(">> phone %s is requesting again after %.1f s of silence", curIP.toString().c_str(), (now - lastHttpMs) / 1000.0);
    else if (!(curIP == lastHttpIP))                 logEvent("request from a different device: %s", curIP.toString().c_str());
    lastHttpMs = now; lastHttpIP = curIP; httpSilentWarned = false;
  }
  if (d >= STALL_WARN_MS) { hSlowFlag = true; logEvent("!! SLOW HTTP %s took %lu ms", EP_NAME[ep], (unsigned long)d); }
}

void jsonEsc(String& o, const char* s) {
  for (; *s; s++) {
    char c = *s;
    if (c == '"' || c == '\\') o += '\'';
    else if ((uint8_t)c < 0x20) o += ' ';
    else o += c;
  }
}
void jsonFloat(String& o, float f) { if (std::isfinite(f)) addf(o, "%.2f", f); else o += "null"; }

int   phonesNow = 0, curCh = AP_CHANNEL;
float rate1s = 0;

const char DEBUG_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Knee DEBUG</title>
<style>
*{box-sizing:border-box}body{margin:0;padding:12px;background:#0b0f14;color:#e6edf3;font:14px -apple-system,Segoe UI,Roboto,sans-serif}
h1{font-size:16px;margin:0 0 10px}.g{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.c{background:#141a21;border:1px solid #2a323d;border-radius:10px;padding:10px;margin-bottom:8px}
.l{font-size:11px;color:#8b949e;text-transform:uppercase;letter-spacing:.5px}
.b{font-size:19px;font-weight:700;margin:4px 0}.s{font-size:11px;color:#8b949e}
.ok{color:#34d399}.bad{color:#f85149}.wa{color:#f0a020}
table{width:100%;border-collapse:collapse;font-size:12px}td{padding:3px 4px;border-bottom:1px solid #1f2630;vertical-align:top}td:first-child{color:#8b949e;white-space:nowrap}
#log{font:11px ui-monospace,Menlo,Consolas,monospace;white-space:pre-wrap;word-break:break-all;height:42vh;overflow:auto;background:#000;border-radius:8px;padding:8px}
</style></head><body>
<h1>Knee Monitor &mdash; DEBUG</h1>
<div class="g">
 <div class="c"><div class="l">Phone &harr; ESP</div><div id="pl" class="b">...</div><div id="pd" class="s"></div></div>
 <div class="c"><div class="l">Sensor &rarr; ESP</div><div id="sl" class="b">...</div><div id="sd" class="s"></div></div>
</div>
<div class="c"><table id="t"></table></div>
<div class="c"><div class="l">Event log (ESP + phone) &mdash; screenshot this</div><div id="log"></div></div>
<script>
var sent=0,okN=0,fail=0,cf=0,latSum=0,latMax=0,lastOk=0,since=0,busy=false,prevUp=0,lines=[];
function $(i){return document.getElementById(i)}
function ts(){return new Date().toTimeString().slice(0,8)}
function log(s,c){lines.push('<span class="'+(c||'')+'">'+s.replace(/&/g,'&amp;').replace(/</g,'&lt;')+'</span>');
 if(lines.length>400)lines.shift();var L=$('log'),bot=L.scrollTop+L.clientHeight>=L.scrollHeight-30;
 L.innerHTML=lines.join('\n');if(bot)L.scrollTop=L.scrollHeight;}
function cls(m){return m.indexOf('!!')>=0?'bad':(m.indexOf('HINT')>=0?'wa':(m.indexOf('>>')>=0?'ok':''))}
function row(k,v){return '<tr><td>'+k+'</td><td>'+v+'</td></tr>'}
function poll(){
 if(busy)return;busy=true;sent++;
 var ctl=window.AbortController?new AbortController():null,t0=Date.now();
 var to=setTimeout(function(){if(ctl)ctl.abort()},2000);
 fetch('/data?since='+since,{cache:'no-store',signal:ctl?ctl.signal:undefined})
 .then(function(r){return r.json()})
 .then(function(d){var ms=Date.now()-t0;okN++;latSum+=ms;if(ms>latMax)latMax=ms;
   if(cf>0)log(ts()+' [phone] ESP answers again after '+cf+' failed requests','ok');
   if(ms>1000)log(ts()+' [phone] slow answer: '+ms+' ms','wa');
   cf=0;lastOk=Date.now();render(d,ms);})
 .catch(function(e){fail++;cf++;
   if(cf==1||cf%10==0)log(ts()+' [phone] !! request to ESP FAILED x'+cf+' ('+(e&&e.name=='AbortError'?'no answer in 2 s':(e&&e.message))+')','bad');})
 .then(function(){clearTimeout(to);busy=false;showLink();});
}
function showLink(){var p=$('pl');
 if(cf==0&&lastOk){p.textContent='OK';p.className='b ok'}else if(!lastOk&&cf==0){p.textContent='...';p.className='b'}else{p.textContent='NO ANSWER x'+cf;p.className='b bad'}
 $('pd').textContent='sent '+sent+' / ok '+okN+' / fail '+fail+' | avg '+(okN?Math.round(latSum/okN):0)+' ms, max '+latMax+' ms'+(lastOk?' | last ok '+((Date.now()-lastOk)/1000).toFixed(1)+' s ago':'');}
function render(d,ms){
 if(prevUp&&d.up<prevUp)log(ts()+' !! ESP REBOOTED (uptime went back to '+(d.up/1000).toFixed(1)+' s)','bad');
 prevUp=d.up;
 for(var i=0;i<d.ev.length;i++){var e=d.ev[i];log('ESP '+(e[1]/1000).toFixed(3)+'s  '+e[2],cls(e[2]));since=e[0]+1;}
 var s=$('sl');
 if(d.linked){s.textContent='OK '+d.rate.toFixed(1)+'/s';s.className='b ok'}
 else if(d.ok==0){s.textContent=d.bad?'WRONG SIZE':'NO DATA EVER';s.className='b bad'}
 else{s.textContent='LOST';s.className='b bad'}
 $('sd').textContent=d.age<0?'no packet received yet':'last packet '+(d.age/1000).toFixed(1)+' s ago';
 $('t').innerHTML=
  row('ESP uptime',(d.up/1000).toFixed(1)+' s')+
  row('Good packets',d.ok+' ('+d.rate.toFixed(1)+'/s)')+
  row('Wrong-size packets',d.bad+(d.bad?' (last len '+d.badLen+', need '+d.need+')':''))+
  row('Signal (RSSI)',d.age<0?'-':d.rssi+' dBm')+
  row('Sender',d.src+'<br>&rarr; '+d.dst)+
  row('Values',d.k.join(' / '))+
  row('Phones on Wi-Fi',d.phones)+
  row('Wi-Fi channel',d.ch+(d.ch!=d.wantCh?' !! expected '+d.wantCh:''))+
  row('ESP free memory',Math.round(d.heap/1024)+' KB')+
  row('ESP slowest loop (2 s)',d.loopMax+' ms')+
  row('This request',ms+' ms');}
document.addEventListener('visibilitychange',function(){log(ts()+' [phone] page is now '+document.visibilityState+(document.visibilityState=='hidden'?' (browser pauses polling!)':''),'wa')});
window.addEventListener('offline',function(){log(ts()+' [phone] !! browser says OFFLINE','bad')});
window.addEventListener('online',function(){log(ts()+' [phone] browser says online','ok')});
log(ts()+' [phone] debug page loaded','ok');
fetch('/time?t='+Math.floor(Date.now()/1000)+'&ua='+encodeURIComponent(navigator.userAgent)).catch(function(){});
setInterval(poll,500);setInterval(showLink,1000);poll();
</script></body></html>)rawliteral";

void handleRoot() {
  hBegin();
  server.send_P(200, "text/html", DEBUG_PAGE);
  hEnd(EP_ROOT);
}

void handleData() {
  hBegin();
  unsigned long now = millis();
  RxState s = snapRx(false);
  bool linked = s.lastGoodMs && now - s.lastGoodMs < SENSOR_TIMEOUT_MS;
  uint32_t since = server.hasArg("since") ? strtoul(server.arg("since").c_str(), NULL, 10) : 0;
  char src[18], dst[18]; macStr(s.src, src); macStr(s.dst, dst);

  String j; j.reserve(3500);
  addf(j, "{\"up\":%lu,\"linked\":%s,\"ok\":%lu,\"bad\":%lu,\"badLen\":%d,\"need\":%d,\"rate\":%.1f,\"age\":%ld,\"rssi\":%d,",
       now, linked ? "true" : "false", (unsigned long)s.ok, (unsigned long)s.bad, s.badLen, EXPECTED_LEN, rate1s,
       s.total ? (long)(now - s.lastMs) : -1L, s.rssi);
  addf(j, "\"src\":\"%s\",\"dst\":\"%s (%s)\",\"k\":[", s.total ? src : "-", s.total ? dst : "-", s.total ? dstKind(s.dst) : "-");
  jsonFloat(j, s.v.knee1); j += ','; jsonFloat(j, s.v.knee2); j += ','; jsonFloat(j, s.v.knee3); j += ','; jsonFloat(j, s.v.knee4);
  addf(j, "],\"phones\":%d,\"heap\":%lu,\"loopMax\":%lu,\"ch\":%d,\"wantCh\":%d,\"ev\":[",
       phonesNow, (unsigned long)ESP.getFreeHeap(),
       (unsigned long)max(secDns.maxWin, max(secHttp.maxWin, secCsv.maxWin)), curCh, AP_CHANNEL);
  uint32_t n = evCount;
  uint32_t from = since;
  if (n > EV_N && from < n - EV_N) from = n - EV_N;
  int sentN = 0;
  for (uint32_t id = from; id < n && sentN < 25; id++) {
    Ev e;
    if (!getEvent(id, e)) continue;
    addf(j, "%s[%lu,%lu,\"", sentN ? "," : "", (unsigned long)e.id, (unsigned long)e.ms);
    jsonEsc(j, e.msg);
    j += "\"]";
    sentN++;
  }
  j += "]}";
  server.send(200, "application/json", j);
  hEnd(EP_DATA);
}

void handleTime() {
  hBegin();
  if (server.hasArg("t")) {
    uint32_t t = strtoul(server.arg("t").c_str(), NULL, 10);
    if (t > 1700000000UL) bootEpoch = t - millis() / 1000;
  }
  String ua = server.hasArg("ua") ? server.arg("ua") : String("?");
  logEvent(">> PAGE OPENED on phone, time synced. Browser: %.70s", ua.c_str());
  if (keepMode)
    logEvent("HINT: Auto-clear is ON -> the REAL firmware would now rewrite the whole CSV and freeze (see CSV scan at boot)");
  server.send(200, "text/plain", "ok");
  hEnd(EP_TIME);
}

void buildReport(String& o, bool resetWin);
void handleDiag() {
  hBegin();
  String o; o.reserve(9000);
  buildReport(o, false);
  o += "\n---- EVENT LOG (last events) ----\n";
  uint32_t n = evCount;
  for (uint32_t id = (n > EV_N ? n - EV_N : 0); id < n; id++) {
    Ev e; if (getEvent(id, e)) addf(o, "[%9.3f] %s\n", e.ms / 1000.0, e.msg);
  }
  server.send(200, "text/plain", o);
  hEnd(EP_DIAG);
}

void handleNotFound() {
  hBegin();
  if (captiveLoggedWin < 4) {
    captiveLoggedWin++;
    logEvent("captive/404: %s asked http://%.40s%.40s -> %s", curIP.toString().c_str(),
             server.hostHeader().c_str(), server.uri().c_str(), CAPTIVE_PORTAL ? "302 to page" : "404");
  }
#if CAPTIVE_PORTAL
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
#else
  server.send(404, "text/plain", "not found");
#endif
  hEnd(EP_CAPTIVE);
}

// ============================ STORAGE ======================================
uint32_t twN = 0, twSum = 0, twMax = 0;

size_t fileSize(const char* p) {
  if (!fsOk || !LittleFS.exists(p)) return 0;
  File f = LittleFS.open(p, "r"); if (!f) return 0;
  size_t s = f.size(); f.close(); return s;
}

void fmtDate(uint32_t ts, char* out) {
  time_t t = (time_t)ts + 7 * 3600;   // Thailand time
  struct tm tm; gmtime_r(&t, &tm);
  strftime(out, 20, "%Y-%m-%d %H:%M", &tm);
}

// Reads the real CSV exactly like the real purge does (line by line). Does NOT modify it.
void scanRealCsv() {
  Serial.println("\n==== CSV SCAN (read only) ====");
  if (!fsOk) { Serial.println("LittleFS not mounted -> skipped"); return; }
  if (!LittleFS.exists("/knee_data.csv")) { Serial.println("/knee_data.csv does not exist"); return; }
  File f = LittleFS.open("/knee_data.csv", "r");
  if (!f) { Serial.println("!! cannot open /knee_data.csv"); return; }
  size_t sz = f.size();
  Serial.printf("File size   : %u bytes (%.1f KB)\n", (unsigned)sz, sz / 1024.0);
  Serial.println("Reading like the real purge does (stops after 15 s) ...");
  unsigned long t0 = millis();
  char line[96];
  uint32_t rows = 0, preSync = 0, junk = 0, firstTs = 0, lastTs = 0;
  bool cut = false;
  f.readBytesUntil('\n', line, sizeof(line) - 1);           // header
  while (f.available()) {
    int n = f.readBytesUntil('\n', line, sizeof(line) - 1);
    line[n] = 0;
    if (n < 5) { junk++; continue; }
    uint32_t ts = strtoul(line, NULL, 10);
    rows++;
    if (ts < 1700000000UL) preSync++;
    else { if (!firstTs) firstTs = ts; lastTs = ts; }
    if ((rows & 255) == 0) { if (millis() - t0 > 15000) { cut = true; break; } yield(); }
  }
  size_t pos = f.position();
  f.close();
  unsigned long dt = millis() - t0;
  float fullMs = (cut && pos) ? dt * (float)sz / pos : dt;
  Serial.printf("Rows read   : %lu%s (short/junk lines %lu)\n", (unsigned long)rows, cut ? " (stopped early)" : "", (unsigned long)junk);
  Serial.printf("Rows w/o real time (written before phone synced time): %lu\n", (unsigned long)preSync);
  if (firstTs) {
    char a[20], b[20]; fmtDate(firstTs, a); fmtDate(lastTs, b);
    Serial.printf("Time span   : %s  ->  %s  (%.1f days)\n", a, b, (lastTs - firstTs) / 86400.0);
  }
  Serial.printf("Read time   : %lu ms%s\n", dt, cut ? "" : " (full file)");
  Serial.printf("Estimated real purge (read+write) blocks the ESP for ~%.1f s\n", fullMs * 2.5 / 1000.0);
  Serial.printf("Estimated /history (1W/1M/1Y) request blocks for ~%.1f s\n", fullMs / 1000.0);
  if (fsOk) Serial.printf("LittleFS    : %u / %u KB used (%.0f%%)\n", (unsigned)(LittleFS.usedBytes() / 1024),
                          (unsigned)(LittleFS.totalBytes() / 1024), 100.0 * LittleFS.usedBytes() / LittleFS.totalBytes());
  if (keepMode && fullMs * 2.5 > 2000)
    Serial.println("!! HINT: Auto-clear is ON and the file is big. In the REAL firmware, right after the phone opens the page,\n"
                   "         the ESP rewrites this whole file and the web page gets NO answers for that long.");
  if (fsOk && LittleFS.usedBytes() > LittleFS.totalBytes() * 0.9)
    Serial.println("!! HINT: flash is almost FULL -> CSV writes will fail / get very slow.");
  logEvent("CSV scan: %u KB, %lu rows, read %lu ms, est. purge block %.1f s", (unsigned)(sz / 1024), (unsigned long)rows, dt, fullMs * 2.5 / 1000.0);
}

void testWrite(const RxState& s) {
  File f = LittleFS.open("/diag_test.csv", "a");
  if (!f) { logEvent("!! test CSV write: open FAILED (flash full / broken FS?)"); return; }
  uint32_t ts = bootEpoch ? bootEpoch + millis() / 1000 : millis() / 1000;
  f.printf("%lu,%.2f,%.2f,%.2f,%.2f\n", (unsigned long)ts, s.v.knee1, s.v.knee2, s.v.knee3, s.v.knee4);
  size_t sz = f.size();
  f.close();
  if (sz > 200000) LittleFS.remove("/diag_test.csv");
}

// ============================ REPORT =======================================
unsigned long reportPrevMs = 0;
uint32_t reportPrevOk = 0;
unsigned long linkUpMs = 0, linkLostMs = 0;
bool wasLinked = false;

void buildReport(String& o, bool resetWin) {
  unsigned long now = millis();
  RxState s = snapRx(resetWin);
  float secs = (now - reportPrevMs) / 1000.0f; if (secs <= 0.01f) secs = 0.01f;
  float rate = (s.ok - reportPrevOk) / secs;
  bool linked = s.lastGoodMs && now - s.lastGoodMs < SENSOR_TIMEOUT_MS;
  char src[18], dst[18]; macStr(s.src, src); macStr(s.dst, dst);

  addf(o, "\n---- REPORT  up %.1f s  (last %.1f s) ------------------------------------\n", now / 1000.0, secs);

  // SENSOR
  addf(o, "SENSOR : %s | good %lu (+%lu, %.1f pkt/s) | wrong-size %lu | NaN %lu\n",
       linked ? "LINKED" : (s.ok ? "!! LOST" : (s.bad ? "!! ONLY WRONG-SIZE PACKETS" : "!! NOTHING RECEIVED YET")),
       (unsigned long)s.ok, (unsigned long)(s.ok - reportPrevOk), rate, (unsigned long)s.bad, (unsigned long)s.nan);
  if (s.total) {
    addf(o, "         last packet %lu ms ago | longest gap %lu ms | RSSI %d dBm (min %d / max %d)\n",
         now - s.lastMs, (unsigned long)s.maxGapWin, s.rssi,
         s.rssiMinWin == 127 ? s.rssi : s.rssiMinWin, s.rssiMaxWin == -127 ? s.rssi : s.rssiMaxWin);
    addf(o, "         from %s -> to %s (%s)\n", src, dst, dstKind(s.dst));
    if (s.ok) addf(o, "         values AL %.2f | AM %.2f | PL %.2f | PM %.2f\n", s.v.knee1, s.v.knee2, s.v.knee3, s.v.knee4);
    if (nSenders > 1) {
      o += "         senders:";
      for (int i = 0; i < nSenders; i++) { char m[18]; macStr(senders[i].mac, m); addf(o, " %s(%lu)", m, (unsigned long)senders[i].n); }
      o += "\n";
    }
  }

  // PHONES
  wifi_sta_list_t list;
  int nSta = 0;
  if (esp_wifi_ap_get_sta_list(&list) == ESP_OK) nSta = list.num;
  addf(o, "PHONE  : %d on Wi-Fi", nSta);
  for (int i = 0; i < nSta && i < 4; i++) { char m[18]; macStr(list.sta[i].mac, m); addf(o, " [%s %d dBm]", m, list.sta[i].rssi); }
  addf(o, " | connects %lu, disconnects %lu", (unsigned long)phoneConnects, (unsigned long)phoneDisconnects);
  if (lastHttpMs) addf(o, " | last page request %.1f s ago from %s\n", (now - lastHttpMs) / 1000.0, lastHttpIP.toString().c_str());
  else o += " | page never requested\n";

  // HTTP
  uint32_t tot = 0; for (int i = 0; i < EP_N; i++) tot += epWin[i];
  addf(o, "HTTP   : %lu req (/ %lu, /data %lu, /time %lu, /diag %lu, captive/404 %lu) | slowest %lu ms (%s)\n",
       (unsigned long)tot, (unsigned long)epWin[EP_ROOT], (unsigned long)epWin[EP_DATA], (unsigned long)epWin[EP_TIME],
       (unsigned long)epWin[EP_DIAG], (unsigned long)epWin[EP_CAPTIVE], (unsigned long)httpSlowWin, httpSlowName);

  // LOOP
  addf(o, "LOOP   : %.0f loops/s | slowest now: dns %lu, http %lu, csv %lu ms | worst ever: dns %lu, http %lu, csv %lu ms\n",
       loopsWin / secs, (unsigned long)secDns.maxWin, (unsigned long)secHttp.maxWin, (unsigned long)secCsv.maxWin,
       (unsigned long)secDns.maxEver, (unsigned long)secHttp.maxEver, (unsigned long)secCsv.maxEver);

  // WIFI
  uint8_t pc; wifi_second_chan_t sc; esp_wifi_get_channel(&pc, &sc);
  wifi_ps_type_t ps = WIFI_PS_NONE; esp_wifi_get_ps(&ps);
  wifi_mode_t md = WiFi.getMode();
  addf(o, "WIFI   : mode %s | channel %d%s | power-save %s | tx %.1f dBm | probe requests %lu\n",
       md == WIFI_AP_STA ? "AP+STA" : md == WIFI_AP ? "AP" : md == WIFI_STA ? "STA" : "OFF",
       pc, pc != AP_CHANNEL ? " !! (expected 1)" : "",
       ps == WIFI_PS_NONE ? "OFF" : ps == WIFI_PS_MIN_MODEM ? "MIN_MODEM" : "MAX_MODEM",
       (int)WiFi.getTxPower() / 4.0, (unsigned long)probeReq);

  // MEMORY
  addf(o, "MEMORY : free %lu KB | lowest ever %lu KB | biggest block %lu KB\n",
       (unsigned long)ESP.getFreeHeap() / 1024, (unsigned long)ESP.getMinFreeHeap() / 1024, (unsigned long)ESP.getMaxAllocHeap() / 1024);

  // STORAGE
  if (fsOk) {
    addf(o, "STORAGE: flash %u/%u KB | knee_data.csv %u KB | Auto-clear %s",
         (unsigned)(LittleFS.usedBytes() / 1024), (unsigned)(LittleFS.totalBytes() / 1024),
         (unsigned)(fileSize("/knee_data.csv") / 1024), keepMode ? "ON" : "off");
    if (twN) addf(o, " | test write avg %lu ms, max %lu ms", (unsigned long)(twSum / twN), (unsigned long)twMax);
    o += "\n";
  } else o += "STORAGE: !! LittleFS NOT mounted\n";

  // HINTS
  if (now > 10000 && s.total == 0) {
    char a[18], b[18]; macStr(staMac, a); macStr(apMac, b);
    addf(o, "HINT   : NOTHING arrives over ESP-NOW. Sender must use channel %d and send to %s (or %s, or FF:FF:FF:FF:FF:FF).\n", AP_CHANNEL, a, b);
  }
  if (s.bad) addf(o, "HINT   : sender packet is %d bytes, screen expects %d -> those packets are IGNORED. Make the structs identical.\n", s.badLen, EXPECTED_LEN);
  if (s.ok && !linked) o += "HINT   : packets STOPPED arriving over the air (ESP-NOW is received in the Wi-Fi task, a busy loop() cannot cause this) -> check the SENDER.\n";
  if (s.total && s.rssi < -80) o += "HINT   : weak signal from sender (< -80 dBm) -> move closer / check antenna.\n";
  if (nSta > 0 && lastHttpMs && now - lastHttpMs > HTTP_SILENT_MS)
    o += "HINT   : phone is on the Wi-Fi but the page is NOT asking for data -> browser closed/in background, or phone sends traffic over mobile data (turn mobile data off).\n";
  if (nSta > 0 && !lastHttpMs) o += "HINT   : phone connected but never opened the page -> open http://192.168.4.1\n";
  if (httpSlowWin > 1000) o += "HINT   : web server answered very slowly -> phone page will look frozen.\n";
  if (ESP.getFreeHeap() < 30000) o += "HINT   : memory is low.\n";
  o += "--------------------------------------------------------------------------\n";

  if (resetWin) {
    reportPrevMs = now; reportPrevOk = s.ok;
    for (int i = 0; i < EP_N; i++) epWin[i] = 0;
    httpSlowWin = 0; httpSlowName = "-"; captiveLoggedWin = 0;
    secDns.maxWin = secHttp.maxWin = secCsv.maxWin = 0;
    loopsWin = 0;
  }
}

// ============================ LINK WATCH ===================================
void checkLink() {
  unsigned long now = millis();
  RxState s = snapRx(false);
  bool linked = s.lastGoodMs && now - s.lastGoodMs < SENSOR_TIMEOUT_MS;
  if (linked == wasLinked) return;
  wasLinked = linked;
  if (linked) {
    if (linkUpMs == 0) logEvent(">> SENSOR LINK UP");
    else logEvent(">> SENSOR LINK BACK after %.1f s without data", (now - linkLostMs) / 1000.0);
    linkUpMs = now;
    return;
  }
  linkLostMs = now;
  logEvent("!! SENSOR LINK LOST: no valid packet for %lu ms (link was up %.1f s, %lu good packets so far)",
           now - s.lastGoodMs, (s.lastGoodMs - linkUpMs) / 1000.0, (unsigned long)s.ok);
  logEvent("   context: phones=%d  channel=%d  last rssi=%d dBm  last ANY packet %lu ms ago  wrong-size=%lu  heap=%lu KB",
           phonesNow, curCh, s.rssi, now - s.lastMs, (unsigned long)s.bad, (unsigned long)ESP.getFreeHeap() / 1024);
  if (s.lastMs > s.lastGoodMs && now - s.lastMs < SENSOR_TIMEOUT_MS)
    logEvent("   HINT: packets still arrive but WRONG SIZE (len %d, need %d) -> sender struct changed", s.badLen, EXPECTED_LEN);
  else
    logEvent("   HINT: packets really stopped arriving over the air -> the SENDER stopped sending or moved channel");
  if (lastPhoneConnMs && s.lastGoodMs >= lastPhoneConnMs && s.lastGoodMs - lastPhoneConnMs < 15000)
    logEvent("   HINT: it stopped %.1f s after a phone connected to the Wi-Fi", (s.lastGoodMs - lastPhoneConnMs) / 1000.0);
  if (lastHttpMs && phonesNow > 0)
    logEvent("   note: phone page last asked for data %.1f s ago", (now - lastHttpMs) / 1000.0);
  if (s.rssi < -80) logEvent("   HINT: signal was weak (%d dBm)", s.rssi);
  if (curCh != AP_CHANNEL) logEvent("   HINT: screen channel is %d, not %d!", curCh, AP_CHANNEL);
}

// ============================ PER-SECOND CHECKS ============================
unsigned long lastSec = 0; uint32_t prevOk1s = 0; bool heapWarned = false;

void everySecond() {
  unsigned long now = millis();
  RxState s = snapRx(false);
  rate1s = (s.ok - prevOk1s) * 1000.0f / (float)(now - lastSec);
  prevOk1s = s.ok;
  lastSec = now;

  uint8_t pc; wifi_second_chan_t sc; esp_wifi_get_channel(&pc, &sc);
  if (pc != curCh) { logEvent("!! WIFI CHANNEL CHANGED %d -> %d (sender must follow!)", curCh, pc); curCh = pc; }

  int ph = WiFi.softAPgetStationNum();
  if (ph != phonesNow) { logEvent("devices on Wi-Fi: %d -> %d", phonesNow, ph); phonesNow = ph; }

  if (phonesNow > 0 && lastHttpMs && !httpSilentWarned && now - lastHttpMs > HTTP_SILENT_MS) {
    httpSilentWarned = true;
    logEvent("!! PHONE STOPPED ASKING FOR DATA: no request for %.1f s but phone is still on the Wi-Fi", (now - lastHttpMs) / 1000.0);
    logEvent("   HINT: browser closed / went to background, captive-portal window closed, or phone uses mobile data");
  }
  if (!heapWarned && ESP.getFreeHeap() < 30000) { heapWarned = true; logEvent("!! LOW MEMORY: %lu bytes free", (unsigned long)ESP.getFreeHeap()); }
}

// ============================ SERIAL COMMANDS ==============================
void printHelp() {
  Serial.println("Commands: r=report  e=all events  v=verbose on/off  s=scan CSV  h=help");
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') { String o; buildReport(o, false); Serial.print(o); }
    else if (c == 'e') {
      Serial.println("---- ALL STORED EVENTS ----");
      uint32_t n = evCount;
      for (uint32_t id = (n > EV_N ? n - EV_N : 0); id < n; id++) { Ev e; if (getEvent(id, e)) Serial.printf("[%9.3f] %s\n", e.ms / 1000.0, e.msg); }
    }
    else if (c == 'v') { verbose = !verbose; Serial.printf("verbose %s\n", verbose ? "ON (every packet)" : "OFF"); }
    else if (c == 's') scanRealCsv();
    else if (c == 'h') printHelp();
  }
}

uint32_t lastVerboseTotal = 0;
void printVerbose() {
  RxState s = snapRx(false);
  if (s.total == lastVerboseTotal) return;
  lastVerboseTotal = s.total;
  Serial.printf("  pkt #%lu  len-ok=%lu bad=%lu  rssi %d  %.2f %.2f %.2f %.2f\n", (unsigned long)s.total,
                (unsigned long)s.ok, (unsigned long)s.bad, s.rssi, s.v.knee1, s.v.knee2, s.v.knee3, s.v.knee4);
}

// ============================ SETUP ========================================
void setup() {
  Serial.begin(115200);
  delay(1500);
  rx.rssiMinWin = 127; rx.rssiMaxWin = -127;

  Serial.println();
  Serial.println("==========================================================");
  Serial.println("   KNEE MONITOR SCREEN  -  DEBUG FIRMWARE");
  Serial.println("==========================================================");
  esp_reset_reason_t rr = esp_reset_reason();
  Serial.printf("Last reset  : %s\n", resetReasonStr(rr));
  Serial.printf("Chip        : %s rev %d, %d cores, %lu MHz\n", ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash/PSRAM : %lu KB / %lu KB\n", (unsigned long)ESP.getFlashChipSize() / 1024, (unsigned long)ESP.getPsramSize() / 1024);
  Serial.printf("Core        : Arduino-ESP32 %d.%d.%d, ESP-IDF %s\n", ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR,
                ESP_ARDUINO_VERSION_PATCH, esp_get_idf_version());
  Serial.printf("Free memory : %lu KB\n", (unsigned long)ESP.getFreeHeap() / 1024);
  Serial.printf("Expecting   : ESP-NOW packets of %d bytes (4 floats)\n", EXPECTED_LEN);
  Serial.printf("Test config : captive portal %s, wifi sleep %s\n", CAPTIVE_PORTAL ? "ON" : "OFF",
                DISABLE_WIFI_SLEEP ? "DISABLED" : "default");
  logEvent("BOOT - last reset: %s", resetReasonStr(rr));

  // saved settings of the real firmware (read only)
  prefs.begin("knee", true);
  keepMode = prefs.getUChar("keep", 0);
  Serial.printf("Saved prefs : theme=%d lang=%d unit=%d auto=%d graph=%d auto-clear=%d (%s)\n",
                prefs.getUChar("theme", 1), prefs.getUChar("lang", 1), prefs.getUChar("metric", 0), prefs.getBool("auto", true),
                prefs.getUChar("gtype", 0), keepMode,
                keepMode == 0 ? "off" : keepMode == 1 ? "1 week" : keepMode == 2 ? "1 month" : "1 year");
  prefs.end();

  fsOk = LittleFS.begin(false);    // false = never format (keep user data safe)
  if (!fsOk) { Serial.println("!! LittleFS mount FAILED (not formatted?)"); logEvent("!! LittleFS mount FAILED"); }
#if SCAN_REAL_CSV
  scanRealCsv();
#endif
  if (fsOk) LittleFS.remove("/diag_test.csv");

  Serial.println("\n==== WI-FI / ESP-NOW ====");
  WiFi.onEvent(onWiFiEvent);
  WiFi.mode(WIFI_AP_STA);
#if DISABLE_WIFI_SLEEP
  WiFi.setSleep(false);
#endif
  bool apOk = WiFi.softAP(AP_SSID, NULL, AP_CHANNEL);
  Serial.printf("softAP      : %s  SSID \"%s\"  IP %s\n", apOk ? "OK" : "!! FAILED", AP_SSID, WiFi.softAPIP().toString().c_str());
  esp_wifi_get_mac(WIFI_IF_STA, staMac);
  esp_wifi_get_mac(WIFI_IF_AP, apMac);
  char a[18], b[18]; macStr(staMac, a); macStr(apMac, b);
  uint8_t pc; wifi_second_chan_t sc; esp_wifi_get_channel(&pc, &sc);
  curCh = pc;
  Serial.printf("Channel     : %d %s\n", pc, pc == AP_CHANNEL ? "(OK)" : "!! NOT the expected channel");
  Serial.printf("STA MAC     : %s  <- sender usually sends here\n", a);
  Serial.printf("AP  MAC     : %s  (also works)\n", b);

  esp_err_t en = esp_now_init();
  Serial.printf("ESP-NOW init: %s\n", en == ESP_OK ? "OK" : esp_err_to_name(en));
  if (en == ESP_OK) esp_now_register_recv_cb(onDataRecv);
  else logEvent("!! ESP-NOW init FAILED: %s", esp_err_to_name(en));

#if CAPTIVE_PORTAL
  dnsServer.start(53, "*", WiFi.softAPIP());
#endif
  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.on("/time", handleTime);
  server.on("/diag", handleDiag);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("\nREADY. Sender must transmit on channel 1 to the STA MAC above.");
  Serial.println("Phone: connect to \"KneeMonitor\" -> debug page (or http://192.168.4.1, full text at /diag)");
  printHelp();
  Serial.println("==========================================================\n");
  logEvent("ready: ch %d, STA %s, AP %s", pc, a, b);
  lastSec = reportPrevMs = millis();
}

// ============================ LOOP =========================================
unsigned long lastReport = 0, lastTestWrite = 0;

void loop() {
  loopsWin++;
#if CAPTIVE_PORTAL
  TIMED(secDns, dnsServer.processNextRequest());
#endif
  hSlowFlag = false;
  TIMED(secHttp, server.handleClient());

  unsigned long now = millis();

#if TEST_CSV_WRITE
  if (fsOk && wasLinked && now - lastTestWrite >= 1000) {
    lastTestWrite = now;
    RxState s = snapRx(false);
    unsigned long t0 = millis();
    TIMED(secCsv, testWrite(s));
    uint32_t d = millis() - t0;
    twN++; twSum += d; if (d > twMax) twMax = d;
  }
#endif

  checkLink();
  if (now - lastSec >= 1000) everySecond();
  handleSerial();
  if (verbose) printVerbose();
  drainEvents();

  if (now - lastReport >= REPORT_MS) {
    lastReport = now;
    String o; o.reserve(2500);
    buildReport(o, true);
    Serial.print(o);
  }
}