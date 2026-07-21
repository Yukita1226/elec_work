#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "data.h"             

const char* ap_ssid = "KneeMonitor";

WebServer  server(80);
DNSServer  dnsServer;
Preferences prefs;
const byte DNS_PORT = 53;

Recive_data  knee;             
Setting_data settings;        

volatile unsigned long lastRecvTime = 0;
const unsigned long TIMEOUT_MS = 3000;


void loadSettings() {
  prefs.begin("knee", true);
  settings.theme    = (Theme)   prefs.getUChar("theme", dark);
  settings.language = (Language)prefs.getUChar("lang",  english);
  settings.metric   = (Modify)  prefs.getUChar("metric", gram);
  settings.isauto   =           prefs.getBool ("auto",  true);
  prefs.end();
}

void saveSettings() {
  prefs.begin("knee", false);
  prefs.putUChar("theme",  settings.theme);
  prefs.putUChar("lang",   settings.language);
  prefs.putUChar("metric", settings.metric);
  prefs.putBool ("auto",   settings.isauto);
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


const char PAGE[] = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Knee Monitor</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--border:#2d333b;--text:#e6edf3;--muted:#8b949e;--accent:#34d399;--accent-soft:rgba(52,211,153,.15);--danger:#f85149;--danger-soft:rgba(248,81,73,.15);}
body.light{--bg:#f6f8fa;--surface:#fff;--border:#d0d7de;--text:#1f2328;--muted:#656d76;--accent:#059669;--accent-soft:rgba(5,150,105,.12);--danger:#cf222e;--danger-soft:rgba(207,34,46,.1);}
*{box-sizing:border-box;margin:0;padding:0;}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans Thai",sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:20px 16px 40px;transition:background .25s,color .25s;}
.header{display:flex;align-items:center;justify-content:space-between;max-width:640px;margin:0 auto 16px;gap:12px;}
.header h1{font-size:clamp(1.05rem,4vw,1.35rem);font-weight:650;letter-spacing:-.01em;}
.gear{flex:none;width:42px;height:42px;border-radius:12px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.2rem;cursor:pointer;transition:.15s;}
.gear:active{transform:scale(.92);}
.status{display:inline-flex;align-items:center;gap:7px;padding:6px 14px;border-radius:999px;font-size:.82rem;font-weight:550;margin-bottom:18px;}
.status.on{background:var(--accent-soft);color:var(--accent);}
.status.off{background:var(--danger-soft);color:var(--danger);}
.status::before{content:"";width:8px;height:8px;border-radius:50%;background:currentColor;}
.status.on::before{animation:pulse 1.6s infinite;}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.35}}
.wrap{max-width:640px;margin:0 auto;}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px;}
@media(min-width:560px){.grid{grid-template-columns:repeat(4,1fr);}}
.card{background:var(--surface);border:1px solid var(--border);border-radius:16px;padding:18px 16px;display:flex;flex-direction:column;gap:6px;transition:.25s;}
.card .label{font-size:.8rem;color:var(--muted);font-weight:500;}
.card .val{font-size:clamp(1.6rem,7vw,2.1rem);font-weight:700;line-height:1;font-variant-numeric:tabular-nums;white-space:nowrap;letter-spacing:-.02em;}
.card .unit{font-size:.75rem;color:var(--muted);font-weight:500;margin-left:2px;}
.panel{position:fixed;inset:0;background:var(--bg);z-index:10;display:flex;flex-direction:column;padding:20px 16px;transform:translateY(100%);transition:transform .28s cubic-bezier(.4,0,.2,1);}
.panel.open{transform:translateY(0);}
.phead{display:flex;align-items:center;justify-content:space-between;max-width:520px;width:100%;margin:0 auto 4px;}
.phead h2{font-size:1.15rem;font-weight:650;}
.close{width:38px;height:38px;border-radius:10px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.1rem;cursor:pointer;}
.pbody{max-width:520px;width:100%;margin:0 auto;overflow:auto;}
.row{display:flex;align-items:center;justify-content:space-between;gap:16px;padding:16px 2px;border-bottom:1px solid var(--border);}
.row:last-child{border-bottom:none;}
.rlabel{font-size:.95rem;font-weight:500;}
select{appearance:none;-webkit-appearance:none;background:var(--surface);color:var(--text);border:1px solid var(--border);border-radius:10px;padding:10px 34px 10px 12px;font-size:.9rem;cursor:pointer;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 24 24' fill='none' stroke='%238b949e' stroke-width='3'%3E%3Cpath d='M6 9l6 6 6-6'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 12px center;min-width:140px;}
.toggle{position:relative;width:50px;height:28px;flex:none;}
.toggle input{opacity:0;width:0;height:0;}
.track{position:absolute;inset:0;background:var(--border);border-radius:999px;cursor:pointer;transition:.2s;}
.track::before{content:"";position:absolute;height:22px;width:22px;left:3px;top:3px;background:#fff;border-radius:50%;transition:.2s;box-shadow:0 1px 3px rgba(0,0,0,.3);}
.toggle input:checked + .track{background:var(--accent);}
.toggle input:checked + .track::before{transform:translateX(22px);}
.save{margin-top:24px;width:100%;padding:15px;border:none;border-radius:12px;background:var(--accent);color:#fff;font-size:1rem;font-weight:600;cursor:pointer;transition:.15s;}
.save:active{transform:scale(.98);}
</style></head><body>

<div class="header">
 <h1 id="title">Knee Sensor Data</h1>
 <button class="gear" onclick="openPanel()">&#9881;</button>
</div>
<div style="text-align:center"><div id="status" class="status off">...</div></div>
<div class="wrap"><div class="grid" id="grid"></div></div>

<div class="panel" id="panel">
 <div class="phead">
  <h2 id="setTitle">Settings</h2>
  <button class="close" onclick="closePanel()">&#10005;</button>
 </div>
 <div class="pbody">
  <div class="row"><span class="rlabel" id="lblTheme">Theme</span>
   <select id="selTheme"><option value="0">Light</option><option value="1">Dark</option></select></div>
  <div class="row"><span class="rlabel" id="lblAuto">Auto theme</span>
   <label class="toggle"><input type="checkbox" id="chkAuto"><span class="track"></span></label></div>
  <div class="row"><span class="rlabel" id="lblLang">Language</span>
   <select id="selLang"><option value="0">ไทย</option><option value="1">English</option></select></div>
  <div class="row"><span class="rlabel" id="lblUnit">Unit</span>
   <select id="selMetric"><option value="0">Gram (g)</option><option value="1">Newton (N)</option><option value="2">Kilogram (kg)</option></select></div>
  <button class="save" id="btnSave" onclick="saveAndClose()">Save &amp; Close</button>
 </div>
</div>

<script>
const T={
 0:{title:"ข้อมูลเซนเซอร์เข่า",knee:"จุด",settings:"ตั้งค่า",theme:"ธีม",auto:"ธีมอัตโนมัติ",lang:"ภาษา",unit:"หน่วย",save:"บันทึกและปิด",on:"เชื่อมต่อแล้ว",off:"ขาดการเชื่อมต่อ"},
 1:{title:"Knee Sensor Data",knee:"Knee",settings:"Settings",theme:"Theme",auto:"Auto theme",lang:"Language",unit:"Unit",save:"Save & Close",on:"Connected",off:"Disconnected"}
};
const UNITS=[{f:1,s:"g"},{f:0.00980665,s:"N"},{f:0.001,s:"kg"}];
let cur={theme:1,lang:1,metric:0,auto:true};

const applyTheme = () => {let t=cur.theme;if(cur.auto){const h=new Date().getHours();t=(h>=7&&h<19)?0:1;}document.body.className=(t===0)?"light":"";}
const applyLang = () => {const t=T[cur.lang];title.textContent=t.title;setTitle.textContent=t.settings;lblTheme.textContent=t.theme;lblAuto.textContent=t.auto;lblLang.textContent=t.lang;lblUnit.textContent=t.unit;btnSave.textContent=t.save;}
const render = (d) => {const t=T[cur.lang],u=UNITS[cur.metric];status.textContent=d.connected?t.on:t.off;status.className="status "+(d.connected?"on":"off");let h="";d.k.forEach((v,i)=>{const val=(v*u.f).toFixed(2);h+=`<div class="card"><div class="label">${t.knee} ${i+1}</div><div class="val">${val}<span class="unit"> ${u.s}</span></div></div>`;});grid.innerHTML=h;}
const poll = async () => {try{const d=await (await fetch("/data")).json();cur.theme=d.theme;cur.lang=d.lang;cur.metric=d.metric;cur.auto=d.auto;applyTheme();applyLang();render(d);}catch(e){}}
const openPanel = () => {selTheme.value=cur.theme;selLang.value=cur.lang;selMetric.value=cur.metric;chkAuto.checked=cur.auto;panel.classList.add("open");}
const closePanel = () => {panel.classList.remove("open");}
const saveAndClose = async () => {const q=`theme=${selTheme.value}&lang=${selLang.value}&metric=${selMetric.value}&auto=${chkAuto.checked?1:0}`;await fetch("/set?"+q);panel.classList.remove("open");poll();}

setInterval(poll,500);poll();
</script>
</body></html>
)HTML";


void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleData() {
  String j = "{";
  j += "\"k\":[" + String(knee.knee1,2) + "," + String(knee.knee2,2) + ","
                 + String(knee.knee3,2) + "," + String(knee.knee4,2) + "],";
  j += "\"connected\":" + String(isConnected() ? "true" : "false") + ",";
  j += "\"theme\":"  + String((int)settings.theme)    + ",";
  j += "\"lang\":"   + String((int)settings.language) + ",";
  j += "\"metric\":" + String((int)settings.metric)   + ",";
  j += "\"auto\":"   + String(settings.isauto ? "true" : "false");
  j += "}";
  server.send(200, "application/json", j);
}

void handleSet() {
  if (server.hasArg("theme"))  settings.theme    = (Theme)   server.arg("theme").toInt();
  if (server.hasArg("lang"))   settings.language = (Language)server.arg("lang").toInt();
  if (server.hasArg("metric")) settings.metric   = (Modify)  server.arg("metric").toInt();
  if (server.hasArg("auto"))   settings.isauto   = (server.arg("auto").toInt() != 0);
  saveSettings();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  loadSettings();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, NULL, 1);
  Serial.print("AP IP: ");         Serial.println(WiFi.softAPIP());
  Serial.print("Receiver MAC: ");  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.on("/set",  handleSet);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}