#pragma once; 
const char PAGE[] = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Knee Monitor</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--border:#2d333b;--text:#e6edf3;--muted:#8b949e;--accent:#34d399;--warn:#f0a020;--danger:#f85149;--accent-soft:rgba(52,211,153,.15);--danger-soft:rgba(248,81,73,.15);}
body.light{--bg:#f6f8fa;--surface:#fff;--border:#d0d7de;--text:#1f2328;--muted:#656d76;--accent:#059669;--warn:#c77700;--danger:#cf222e;--accent-soft:rgba(5,150,105,.12);--danger-soft:rgba(207,34,46,.1);}
*{box-sizing:border-box;margin:0;padding:0;}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans Thai",sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:18px 16px 34px;transition:background .25s,color .25s;}
.header{display:flex;align-items:center;justify-content:space-between;max-width:520px;margin:0 auto 14px;gap:12px;}
.header h1{font-size:clamp(1.05rem,4vw,1.3rem);font-weight:650;letter-spacing:-.01em;}
.gear{flex:none;width:42px;height:42px;border-radius:12px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.2rem;cursor:pointer;transition:.15s;}
.gear:active{transform:scale(.92);}
.status{display:inline-flex;align-items:center;gap:7px;padding:6px 14px;border-radius:999px;font-size:.82rem;font-weight:550;margin-bottom:6px;}
.status.on{background:var(--accent-soft);color:var(--accent);}
.status.off{background:var(--danger-soft);color:var(--danger);}
.status::before{content:"";width:8px;height:8px;border-radius:50%;background:currentColor;}
.status.on::before{animation:pulse 1.6s infinite;}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.35}}
.wrap{max-width:520px;margin:0 auto;text-align:center;}
.canvas{position:relative;width:min(88vw,360px);aspect-ratio:1;margin:6px auto 0;}
.canvas svg.knee{position:absolute;inset:0;width:100%;height:100%;}
.ori{position:absolute;font-size:.62rem;letter-spacing:1.5px;color:var(--muted);font-weight:600;}
.ori-t{top:-3px;left:50%;transform:translateX(-50%);}
.ori-b{bottom:-3px;left:50%;transform:translateX(-50%);}
.ori-l{left:-3px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%);}
.ori-r{right:-3px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%);}
.sensor{position:absolute;text-align:center;z-index:2;}
.sensor .sl{font-size:.62rem;color:var(--muted);letter-spacing:.5px;}
.sensor .num{font-size:clamp(1.3rem,6vw,1.9rem);font-weight:700;line-height:1;font-variant-numeric:tabular-nums;transition:color .2s;}
.sensor .su{font-size:.68rem;color:var(--muted);}
.s-tl{top:4px;left:2px;} .s-tr{top:4px;right:2px;} .s-bl{bottom:4px;left:2px;} .s-br{bottom:4px;right:2px;}
.dot{position:absolute;width:28px;height:28px;left:50%;top:50%;transform:translate(-50%,-50%);transition:left .25s ease,top .25s ease;pointer-events:none;z-index:3;}
.dot::before{content:"";position:absolute;inset:0;border-radius:50%;background:#22d3ee;box-shadow:0 0 14px 3px rgba(34,211,238,.7);}
.dot::after{content:"";position:absolute;inset:-9px;border:2px solid rgba(34,211,238,.45);border-radius:50%;}
.panel{position:fixed;inset:0;background:var(--bg);z-index:10;display:flex;flex-direction:column;padding:20px 16px;transform:translateY(100%);transition:transform .28s cubic-bezier(.4,0,.2,1);}
.panel.open{transform:translateY(0);}
.phead{display:flex;align-items:center;justify-content:space-between;max-width:460px;width:100%;margin:0 auto 4px;}
.phead h2{font-size:1.15rem;font-weight:650;}
.close{width:38px;height:38px;border-radius:10px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.1rem;cursor:pointer;}
.pbody{max-width:460px;width:100%;margin:0 auto;overflow:auto;}
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
.save{margin-top:24px;width:100%;padding:15px;border:none;border-radius:12px;background:var(--accent);color:#fff;font-size:1rem;font-weight:600;cursor:pointer;}
.save:active{transform:scale(.98);}
</style></head><body>

<div class="header">
 <h1 id="title">Knee Load Monitor</h1>
 <button class="gear" onclick="openPanel()">&#9881;</button>
</div>
<div style="text-align:center"><div id="status" class="status off">...</div></div>

<div class="wrap">
 <div class="canvas">
  <svg class="knee" viewBox="0 0 300 300">
   <path d="M150 52 C198 52 246 78 248 128 C250 172 238 216 196 238 C176 248 124 248 104 238 C62 216 50 172 52 128 C54 78 102 52 150 52 Z" fill="#c9a97e" stroke="#8a6d45" stroke-width="3"/>
   <ellipse cx="108" cy="150" rx="40" ry="58" fill="#b8956a"/>
   <ellipse cx="192" cy="150" rx="40" ry="58" fill="#b8956a"/>
   <ellipse cx="108" cy="146" rx="29" ry="45" fill="#cdb083"/>
   <ellipse cx="192" cy="146" rx="29" ry="45" fill="#cdb083"/>
   <path d="M144 120 L150 152 L150 120 Z" fill="#a37f50"/>
   <path d="M156 120 L150 152 L150 120 Z" fill="#956f45"/>
   <g stroke="rgba(255,255,255,.16)" stroke-width="1.5" fill="none">
    <line x1="150" y1="28" x2="150" y2="272"/>
    <line x1="28" y1="150" x2="272" y2="150"/>
    <circle cx="150" cy="150" r="52"/>
    <circle cx="150" cy="150" r="98"/>
   </g>
  </svg>
  <span class="ori ori-t" id="oriT">ANTERIOR</span>
  <span class="ori ori-b" id="oriB">POSTERIOR</span>
  <span class="ori ori-l" id="oriL">LATERAL</span>
  <span class="ori ori-r" id="oriR">MEDIAL</span>
  <div class="sensor s-tl"><div class="sl">AL</div><div><span class="num" id="v0">0</span><span class="su" id="u0"> g</span></div></div>
  <div class="sensor s-tr"><div class="sl">AM</div><div><span class="num" id="v1">0</span><span class="su" id="u1"> g</span></div></div>
  <div class="sensor s-bl"><div><span class="num" id="v2">0</span><span class="su" id="u2"> g</span></div><div class="sl">PL</div></div>
  <div class="sensor s-br"><div><span class="num" id="v3">0</span><span class="su" id="u3"> g</span></div><div class="sl">PM</div></div>
  <div class="dot" id="dot"></div>
 </div>
</div>

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
const LOAD_WARN=20000, LOAD_DANGER=35000;
const T={
 0:{title:"จอวัดน้ำหนักลงเข่า",settings:"ตั้งค่า",theme:"ธีม",auto:"ธีมอัตโนมัติ",lang:"ภาษา",unit:"หน่วย",save:"บันทึกและปิด",on:"เชื่อมต่อแล้ว",off:"ขาดการเชื่อมต่อ",ant:"ด้านหน้า",post:"ด้านหลัง",med:"ด้านใน",lat:"ด้านนอก"},
 1:{title:"Knee Load Monitor",settings:"Settings",theme:"Theme",auto:"Auto theme",lang:"Language",unit:"Unit",save:"Save & Close",on:"Connected",off:"Disconnected",ant:"ANTERIOR",post:"POSTERIOR",med:"MEDIAL",lat:"LATERAL"}
};
const UNITS=[{f:1,s:"g"},{f:0.00980665,s:"N"},{f:0.001,s:"kg"}];
let cur={theme:1,lang:1,metric:0,auto:true};

  // Indented below to avoid Arduino C++ preprocessor bugs
  function applyTheme(){let t=cur.theme;if(cur.auto){const h=new Date().getHours();t=(h>=7&&h<19)?0:1;}document.body.className=(t===0)?"light":"";}
  function applyLang(){const t=T[cur.lang];title.textContent=t.title;setTitle.textContent=t.settings;lblTheme.textContent=t.theme;lblAuto.textContent=t.auto;lblLang.textContent=t.lang;lblUnit.textContent=t.unit;btnSave.textContent=t.save;oriT.textContent=t.ant;oriB.textContent=t.post;oriL.textContent=t.lat;oriR.textContent=t.med;}
  function render(d){
   const t=T[cur.lang],u=UNITS[cur.metric];
   status.textContent=d.connected?t.on:t.off;status.className="status "+(d.connected?"on":"off");
   const w=d.k;
   for(let i=0;i<4;i++){
    const el=document.getElementById("v"+i);
    el.textContent=(w[i]*u.f).toFixed(1);
    document.getElementById("u"+i).textContent=" "+u.s;
    el.style.color=w[i]>=LOAD_DANGER?"var(--danger)":(w[i]>=LOAD_WARN?"var(--warn)":"var(--accent)");
   }
   const tot=w[0]+w[1]+w[2]+w[3];let nx=0,ny=0;
   if(tot>0){nx=(-w[0]+w[1]-w[2]+w[3])/tot;ny=(-w[0]-w[1]+w[2]+w[3])/tot;}
   dot.style.left=(50+nx*40)+"%";dot.style.top=(50+ny*40)+"%";
  }
  async function poll(){try{const d=await (await fetch("/data")).json();cur.theme=d.theme;cur.lang=d.lang;cur.metric=d.metric;cur.auto=d.auto;applyTheme();applyLang();render(d);}catch(e){}}
  function openPanel(){selTheme.value=cur.theme;selLang.value=cur.lang;selMetric.value=cur.metric;chkAuto.checked=cur.auto;panel.classList.add("open");}
  function closePanel(){panel.classList.remove("open");}
  async function saveAndClose(){const q=`theme=${selTheme.value}&lang=${selLang.value}&metric=${selMetric.value}&auto=${chkAuto.checked?1:0}`;await fetch("/set?"+q);panel.classList.remove("open");poll();}

setInterval(poll,500);poll();
</script>
</body></html>
)HTML";