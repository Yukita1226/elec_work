#pragma once
const char PAGE[] = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Knee Monitor</title>
<style>
:root{--bg:#0b0f14;--surface:#141a21;--surface2:#1b222b;--border:#2a323d;--text:#e6edf3;--muted:#8b949e;--accent:#34d399;--warn:#f0a020;--danger:#f85149;--accent-soft:rgba(52,211,153,.15);--danger-soft:rgba(248,81,73,.15);--c0:#22d3ee;--c1:#fbbf24;--c2:#a78bfa;--c3:#34d399;
/* bone material */
--b1:#fbf4e7;--b2:#eaddc4;--b3:#d5c0a0;--b4:#a98b60;--b5:#8a7047;--bink:#4a3a22;--gloss:.22;--mott:.38;}
body.light{--bg:#f2f4f7;--surface:#fff;--surface2:#f7f9fb;--border:#d0d7de;--text:#1f2328;--muted:#5c6773;--accent:#059669;--warn:#c77700;--danger:#cf222e;--accent-soft:rgba(5,150,105,.12);--danger-soft:rgba(207,34,46,.1);--c0:#0891b2;--c1:#b45309;--c2:#7c3aed;--c3:#059669;}
body.poly{--b1:#fffde8;--b2:#f6ee46;--b3:#ddd400;--b4:#9e9800;--b5:#6f6b00;--bink:#3a3800;--gloss:.55;--mott:.12;}
*{box-sizing:border-box;margin:0;padding:0;}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans Thai",sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:16px 14px 30px;transition:background .25s,color .25s;-webkit-tap-highlight-color:transparent;}
.wrap{max-width:520px;margin:0 auto;}
.header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:12px;}
.header h1{font-size:clamp(1.02rem,4.2vw,1.28rem);font-weight:650;letter-spacing:-.01em;}
.gear{flex:none;width:42px;height:42px;border-radius:12px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.2rem;cursor:pointer;transition:.15s;}
.gear:active{transform:scale(.92);}
.statusbar{text-align:center;margin-bottom:2px;}
.status{display:inline-flex;align-items:center;gap:7px;padding:6px 14px;border-radius:999px;font-size:.8rem;font-weight:550;}
.status.on{background:var(--accent-soft);color:var(--accent);}
.status.off{background:var(--danger-soft);color:var(--danger);}
.status::before{content:"";width:8px;height:8px;border-radius:50%;background:currentColor;}
.status.on::before{animation:pulse 1.6s infinite;}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}

/* ---------- insert view ---------- */
.canvas{position:relative;width:min(94vw,470px);aspect-ratio:320/250;margin:4px auto 0;}
.canvas svg.knee{position:absolute;inset:0;width:100%;height:100%;overflow:visible;}
.ori{position:absolute;font-size:.58rem;letter-spacing:1.6px;color:var(--muted);font-weight:600;}
.ori-t{top:-10px;left:50%;transform:translateX(-50%);}
.ori-b{bottom:-10px;left:50%;transform:translateX(-50%);}
.ori-l{left:-9px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%) rotate(180deg);}
.ori-r{right:-9px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%);}
.dot{position:absolute;width:24px;height:24px;left:50%;top:50%;transform:translate(-50%,-50%);transition:left .22s ease,top .22s ease;pointer-events:none;z-index:3;}
.dot::before{content:"";position:absolute;inset:0;border-radius:50%;background:#22d3ee;box-shadow:0 0 16px 4px rgba(34,211,238,.6);}
.dot::after{content:"";position:absolute;inset:-8px;border:2px solid rgba(34,211,238,.4);border-radius:50%;}
.copnote{text-align:center;font-size:.63rem;color:var(--muted);letter-spacing:1px;margin-top:12px;}

/* svg typography */
.big{font-weight:700;font-size:30px;font-variant-numeric:tabular-nums;paint-order:stroke;stroke:rgba(0,0,0,.45);stroke-width:3px;stroke-linejoin:round;}
.unit{font-size:10px;font-weight:600;letter-spacing:2px;fill:var(--bink);opacity:.75;}
.small{font-weight:700;font-size:13px;font-variant-numeric:tabular-nums;paint-order:stroke;stroke:rgba(0,0,0,.35);stroke-width:2.5px;stroke-linejoin:round;}
.tag{font-size:8.5px;font-weight:700;letter-spacing:1.4px;fill:var(--bink);opacity:.55;}
.etch{fill:none;stroke:var(--bink);opacity:.16;}
 #specA,#specB,#specC{opacity:var(--gloss);}
 #mottle{opacity:var(--mott);}

/* ---------- chart ---------- */
.chartcard{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:12px 12px 10px;margin-top:12px;}
.chead{display:flex;align-items:center;justify-content:space-between;margin-bottom:6px;}
.chead h3{font-size:.8rem;font-weight:600;letter-spacing:.3px;}
.cbtns{display:flex;gap:6px;}
.cbtn{border:1px solid var(--border);background:var(--surface2);color:var(--muted);border-radius:8px;padding:5px 10px;font-size:.7rem;font-weight:600;cursor:pointer;}
.cbtn.active{color:var(--accent);border-color:var(--accent);}
canvas{width:100%;height:176px;display:block;}
.legend{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;margin-top:8px;}
.lg{display:flex;flex-direction:column;align-items:center;gap:2px;font-size:.62rem;color:var(--muted);}
.lg b{display:flex;align-items:center;gap:5px;font-size:.66rem;letter-spacing:.6px;}
.lg i{width:9px;height:3px;border-radius:2px;}
.lg span{font-size:.82rem;font-weight:700;color:var(--text);font-variant-numeric:tabular-nums;}

/* ---------- settings ---------- */
.panel{position:fixed;inset:0;background:var(--bg);z-index:10;display:flex;flex-direction:column;padding:20px 16px;transform:translateY(100%);transition:transform .28s cubic-bezier(.4,0,.2,1);}
.panel.open{transform:translateY(0);}
.phead{display:flex;align-items:center;justify-content:space-between;max-width:460px;width:100%;margin:0 auto 4px;}
.phead h2{font-size:1.15rem;font-weight:650;}
.close{width:38px;height:38px;border-radius:10px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.1rem;cursor:pointer;}
.pbody{max-width:460px;width:100%;margin:0 auto;overflow:auto;}
.row{display:flex;align-items:center;justify-content:space-between;gap:16px;padding:15px 2px;border-bottom:1px solid var(--border);}
.row:last-of-type{border-bottom:none;}
.rlabel{font-size:.95rem;font-weight:500;}
.rlabel small{display:block;font-size:.72rem;color:var(--muted);font-weight:400;margin-top:2px;}
select{appearance:none;-webkit-appearance:none;background:var(--surface);color:var(--text);border:1px solid var(--border);border-radius:10px;padding:10px 34px 10px 12px;font-size:.9rem;cursor:pointer;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 24 24' fill='none' stroke='%238b949e' stroke-width='3'%3E%3Cpath d='M6 9l6 6 6-6'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 12px center;min-width:140px;}
.toggle{position:relative;width:50px;height:28px;flex:none;}
.toggle input{opacity:0;width:0;height:0;}
.track{position:absolute;inset:0;background:var(--border);border-radius:999px;cursor:pointer;transition:.2s;}
.track::before{content:"";position:absolute;height:22px;width:22px;left:3px;top:3px;background:#fff;border-radius:50%;transition:.2s;box-shadow:0 1px 3px rgba(0,0,0,.3);}
.toggle input:checked + .track{background:var(--accent);}
.toggle input:checked + .track::before{transform:translateX(22px);}
.save{margin-top:22px;width:100%;padding:15px;border:none;border-radius:12px;background:var(--accent);color:#fff;font-size:1rem;font-weight:600;cursor:pointer;}
.save:active{transform:scale(.98);}
@media (prefers-reduced-motion:reduce){*{animation:none!important;transition:none!important;}}
</style></head><body>

<div class="wrap">
 <div class="header">
  <h1 id="title">Knee Load Monitor</h1>
  <button class="gear" onclick="openPanel()">&#9881;</button>
 </div>
 <div class="statusbar"><div id="status" class="status off">...</div></div>

 <div class="canvas">
  <svg class="knee" viewBox="0 0 320 250">
   <defs>
    <linearGradient id="gBody" x1="18%" y1="0%" x2="82%" y2="100%">
     <stop offset="0"   stop-color="#fbf4e7" style="stop-color:var(--b1)"/>
     <stop offset="42%" stop-color="#eaddc4" style="stop-color:var(--b2)"/>
     <stop offset="100%" stop-color="#d5c0a0" style="stop-color:var(--b3)"/>
    </linearGradient>
    <linearGradient id="gEdge" x1="0%" y1="0%" x2="0%" y2="100%">
     <stop offset="0"   stop-color="#fbf4e7" style="stop-color:var(--b1)"/>
     <stop offset="100%" stop-color="#a98b60" style="stop-color:var(--b4)"/>
    </linearGradient>
    <radialGradient id="gDish" cx="56%" cy="70%" r="72%">
     <stop offset="0"   stop-color="#fbf4e7" style="stop-color:var(--b1)"/>
     <stop offset="45%" stop-color="#d5c0a0" style="stop-color:var(--b3)"/>
     <stop offset="100%" stop-color="#a98b60" style="stop-color:var(--b4)"/>
    </radialGradient>
    <radialGradient id="gVig" cx="50%" cy="42%" r="62%">
     <stop offset="60%" stop-color="#000" stop-opacity="0"/>
     <stop offset="100%" stop-color="#000" stop-opacity=".35"/>
    </radialGradient>
    <linearGradient id="gSpec" x1="0" y1="0" x2="0" y2="1">
     <stop offset="0" stop-color="#fff" stop-opacity=".95"/>
     <stop offset="100%" stop-color="#fff" stop-opacity="0"/>
    </linearGradient>
    <filter id="fGrain" x="-5%" y="-5%" width="110%" height="110%">
     <feTurbulence type="fractalNoise" baseFrequency="0.7 0.9" numOctaves="4" seed="7" result="n"/>
     <feColorMatrix in="n" type="saturate" values="0"/>
     <feComponentTransfer><feFuncA type="linear" slope="0.28"/></feComponentTransfer>
    </filter>
    <filter id="fBlur6"><feGaussianBlur stdDeviation="6"/></filter>
    <filter id="fBlur3"><feGaussianBlur stdDeviation="3"/></filter>
    <filter id="fDrop" x="-25%" y="-25%" width="150%" height="160%">
     <feGaussianBlur stdDeviation="9"/>
    </filter>
    <path id="pOut" d="M42 56 C66 28 102 20 120 28 C124 37 132 37 136 28 C146 22 174 22 184 28 C188 37 196 37 200 28 C218 20 254 28 278 56 C306 90 304 154 282 188 C268 210 236 224 212 216 C196 211 186 200 182 186 C178 170 172 152 160 146 C148 152 142 170 138 186 C134 200 124 211 108 216 C84 224 52 210 38 188 C16 154 14 90 42 56 Z"/>
    <clipPath id="clipBody"><path d="M42 56 C66 28 102 20 120 28 C124 37 132 37 136 28 C146 22 174 22 184 28 C188 37 196 37 200 28 C218 20 254 28 278 56 C306 90 304 154 282 188 C268 210 236 224 212 216 C196 211 186 200 182 186 C178 170 172 152 160 146 C148 152 142 170 138 186 C134 200 124 211 108 216 C84 224 52 210 38 188 C16 154 14 90 42 56 Z"/></clipPath>
    <clipPath id="clipDishL"><ellipse cx="92" cy="122" rx="52" ry="62"/></clipPath>
    <clipPath id="clipDishM"><ellipse cx="228" cy="122" rx="52" ry="62"/></clipPath>
   </defs>

   <!-- cast shadow -->
   <use href="#pOut" xlink:href="#pOut" fill="#000" opacity=".45" transform="translate(4,10)" filter="url(#fDrop)"/>

   <!-- bevelled outer edge, then top surface inset slightly -->
   <use href="#pOut" xlink:href="#pOut" fill="url(#gEdge)"/>
   <g transform="translate(160,124) scale(.965) translate(-160,-124)">
    <use href="#pOut" xlink:href="#pOut" fill="url(#gBody)"/>
   </g>

   <g clip-path="url(#clipBody)">
    <!-- porous mottling -->
    <rect id="mottle" x="0" y="0" width="320" height="250" fill="#e6d3b4" filter="url(#fGrain)"/>

    <!-- lateral dish (left) -->
    <ellipse cx="92" cy="122" rx="52" ry="62" fill="url(#gDish)"/>
    <g clip-path="url(#clipDishL)">
     <ellipse cx="92" cy="122" rx="52" ry="62" fill="none" stroke="#6d5837" stroke-width="14" opacity=".45" filter="url(#fBlur6)"/>
     <ellipse cx="88" cy="150" rx="34" ry="26" fill="#fff" opacity=".16" filter="url(#fBlur6)"/>
     <g class="etch" stroke-width="1.4">
      <path d="M56 86 L128 104 L70 172 Z"/>
      <path d="M64 96 L116 109 L74 158 Z"/>
      <path d="M56 86 L74 172"/><path d="M64 130 L118 106"/>
     </g>
    </g>
    <ellipse id="heatL" cx="92" cy="122" rx="52" ry="62" fill="#f85149" opacity="0"/>
    <ellipse cx="92" cy="122" rx="52" ry="62" fill="none" stroke="#fff" stroke-width="2" opacity=".22"/>

    <!-- medial dish (right) -->
    <ellipse cx="228" cy="122" rx="52" ry="62" fill="url(#gDish)"/>
    <g clip-path="url(#clipDishM)">
     <ellipse cx="228" cy="122" rx="52" ry="62" fill="none" stroke="#6d5837" stroke-width="14" opacity=".45" filter="url(#fBlur6)"/>
     <ellipse cx="232" cy="150" rx="34" ry="26" fill="#fff" opacity=".16" filter="url(#fBlur6)"/>
     <g class="etch" stroke-width="1.4">
      <path d="M264 86 L192 104 L250 172 Z"/>
      <path d="M256 96 L204 109 L246 158 Z"/>
      <path d="M264 86 L246 172"/><path d="M256 130 L202 106"/>
     </g>
    </g>
    <ellipse id="heatM" cx="228" cy="122" rx="52" ry="62" fill="#f85149" opacity="0"/>
    <ellipse cx="228" cy="122" rx="52" ry="62" fill="none" stroke="#fff" stroke-width="2" opacity=".22"/>

    <!-- central eminence between the dishes -->
    <path d="M150 44 C143 78 143 120 152 148 C157 162 163 162 168 148 C177 120 177 78 170 44 Z"
          fill="url(#gEdge)" opacity=".9"/>
    <path d="M154 52 C148 84 148 120 156 146" fill="none" stroke="#fff" stroke-width="2.6" opacity=".38"/>
    <path d="M166 52 C172 84 172 120 164 146" fill="none" stroke="#000" stroke-width="2" opacity=".18"/>

    <!-- gloss highlights -->
    <ellipse id="specA" cx="86" cy="52" rx="46" ry="15" fill="url(#gSpec)" transform="rotate(-13 86 52)" filter="url(#fBlur3)"/>
    <ellipse id="specB" cx="238" cy="54" rx="42" ry="13" fill="url(#gSpec)" transform="rotate(11 238 54)" filter="url(#fBlur3)"/>
    <ellipse id="specC" cx="60" cy="150" rx="10" ry="52" fill="url(#gSpec)" transform="rotate(6 60 150)" filter="url(#fBlur6)"/>

    <!-- vignette -->
    <rect x="0" y="0" width="320" height="250" fill="url(#gVig)"/>
   </g>

   <!-- crisp outline over everything -->
   <use href="#pOut" xlink:href="#pOut" fill="none" stroke="#8a7047" style="stroke:var(--b5)" stroke-width="2.2"/>

   <!-- sensor pads + readouts -->
   <g id="pads">
    <circle id="p0" cx="92"  cy="84"  r="6.5" fill="#34d399" stroke="rgba(0,0,0,.55)" stroke-width="2"/>
    <circle id="p1" cx="228" cy="84"  r="6.5" fill="#34d399" stroke="rgba(0,0,0,.55)" stroke-width="2"/>
    <circle id="p2" cx="92"  cy="172" r="6.5" fill="#34d399" stroke="rgba(0,0,0,.55)" stroke-width="2"/>
    <circle id="p3" cx="228" cy="172" r="6.5" fill="#34d399" stroke="rgba(0,0,0,.55)" stroke-width="2"/>
   </g>
   <g text-anchor="middle">
    <text class="tag" x="70"  y="70">AL</text>
    <text class="tag" x="250" y="70">AM</text>
    <text class="tag" x="70"  y="190">PL</text>
    <text class="tag" x="250" y="190">PM</text>
    <text class="small" id="s0" x="104" y="70"  fill="#34d399">0</text>
    <text class="small" id="s1" x="216" y="70"  fill="#34d399">0</text>
    <text class="small" id="s2" x="104" y="190" fill="#34d399">0</text>
    <text class="small" id="s3" x="216" y="190" fill="#34d399">0</text>
    <text class="big"   id="bigL" x="92"  y="132" fill="#34d399">0</text>
    <text class="big"   id="bigM" x="228" y="132" fill="#34d399">0</text>
    <text class="unit"  id="uL" x="92"  y="150">g</text>
    <text class="unit"  id="uM" x="228" y="150">g</text>
   </g>
  </svg>

  <span class="ori ori-t" id="oriT">ANTERIOR</span>
  <span class="ori ori-b" id="oriB">POSTERIOR</span>
  <span class="ori ori-l" id="oriL">LATERAL</span>
  <span class="ori ori-r" id="oriR">MEDIAL</span>
  <div class="dot" id="dot"></div>
 </div>
 <div class="copnote" id="copnote">CENTER OF LOAD</div>

 <div class="chartcard">
  <div class="chead">
   <h3 id="chTitle">Load over time</h3>
   <div class="cbtns">
    <button class="cbtn" id="btnPause" onclick="togglePause()">Pause</button>
    <button class="cbtn" id="btnClear" onclick="clearHist()">Clear</button>
   </div>
  </div>
  <canvas id="chart"></canvas>
  <div class="legend">
   <div class="lg"><b><i style="background:var(--c0)"></i>AL</b><span id="lv0">0</span></div>
   <div class="lg"><b><i style="background:var(--c1)"></i>AM</b><span id="lv1">0</span></div>
   <div class="lg"><b><i style="background:var(--c2)"></i>PL</b><span id="lv2">0</span></div>
   <div class="lg"><b><i style="background:var(--c3)"></i>PM</b><span id="lv3">0</span></div>
  </div>
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
  <div class="row"><span class="rlabel" id="lblMat">Material<small id="lblMatSub">Stored on this phone only</small></span>
   <select id="selMat" onchange="applyMat(this.value)"><option value="0">Bone</option><option value="1">Trial insert</option></select></div>
  <button class="save" id="btnSave" onclick="saveAndClose()">Save &amp; Close</button>
 </div>
</div>

<script>
const LOAD_WARN=20000, LOAD_DANGER=35000;
const T={
 0:{title:"จอวัดน้ำหนักลงเข่า",settings:"ตั้งค่า",theme:"ธีม",auto:"ธีมอัตโนมัติ",lang:"ภาษา",unit:"หน่วย",save:"บันทึกและปิด",on:"เชื่อมต่อแล้ว",off:"ขาดการเชื่อมต่อ",ant:"ด้านหน้า",post:"ด้านหลัง",med:"ด้านใน",lat:"ด้านนอก",cop:"จุดศูนย์กลางแรงกด",chart:"กราฟแรงกดตามเวลา",pause:"หยุด",resume:"เล่นต่อ",clear:"ล้าง",mat:"พื้นผิว",matsub:"บันทึกในเครื่องนี้เท่านั้น",m0:"กระดูก",m1:"แผ่นทดลอง"},
 1:{title:"Knee Load Monitor",settings:"Settings",theme:"Theme",auto:"Auto theme",lang:"Language",unit:"Unit",save:"Save & Close",on:"Connected",off:"Disconnected",ant:"ANTERIOR",post:"POSTERIOR",med:"MEDIAL",lat:"LATERAL",cop:"CENTER OF LOAD",chart:"Load over time",pause:"Pause",resume:"Resume",clear:"Clear",mat:"Material",matsub:"Stored on this phone only",m0:"Bone",m1:"Trial insert"}
};
const UNITS=[{f:1,s:"g",d:0},{f:0.00980665,s:"N",d:1},{f:0.001,s:"kg",d:2}];
let cur={theme:1,lang:1,metric:0,auto:true};

  // Indented below to avoid Arduino C++ preprocessor bugs
  const HIST=120;
  let hist=[[],[],[],[]], paused=false, mat=0;

  try{mat=parseInt(localStorage.getItem("kneeMat")||"0",10)||0;}catch(e){}

  function applyMat(v){mat=parseInt(v,10)||0;document.body.classList.toggle("poly",mat===1);
   try{localStorage.setItem("kneeMat",mat);}catch(e){}}
  function applyTheme(){let t=cur.theme;if(cur.auto){const h=new Date().getHours();t=(h>=7&&h<19)?0:1;}
   document.body.classList.toggle("light",t===0);document.body.classList.toggle("poly",mat===1);}
  function applyLang(){const t=T[cur.lang];
   title.textContent=t.title;setTitle.textContent=t.settings;lblTheme.textContent=t.theme;lblAuto.textContent=t.auto;
   lblLang.textContent=t.lang;lblUnit.textContent=t.unit;btnSave.textContent=t.save;
   lblMat.firstChild.nodeValue=t.mat;lblMatSub.textContent=t.matsub;
   selMat.options[0].text=t.m0;selMat.options[1].text=t.m1;
   oriT.textContent=t.ant;oriB.textContent=t.post;oriL.textContent=t.lat;oriR.textContent=t.med;
   copnote.textContent=t.cop;chTitle.textContent=t.chart;
   btnPause.textContent=paused?t.resume:t.pause;btnClear.textContent=t.clear;}

  function rawColor(v){const cs=getComputedStyle(document.body);
   return cs.getPropertyValue(v>=LOAD_DANGER?"--danger":(v>=LOAD_WARN?"--warn":"--accent")).trim();}

  function render(d){
   const t=T[cur.lang],u=UNITS[cur.metric];
   status.textContent=d.connected?t.on:t.off;status.className="status "+(d.connected?"on":"off");
   const w=d.k;

   for(let i=0;i<4;i++){
    const c=rawColor(w[i]),txt=(w[i]*u.f).toFixed(u.d);
    const s=document.getElementById("s"+i);s.textContent=txt;s.setAttribute("fill",c);
    document.getElementById("p"+i).setAttribute("fill",c);
    document.getElementById("lv"+i).textContent=txt;
   }

   // compartment totals: lateral = AL+PL, medial = AM+PM
   const lat=w[0]+w[2], med=w[1]+w[3];
   bigL.textContent=(lat*u.f).toFixed(u.d);bigL.setAttribute("fill",rawColor(lat/2));
   bigM.textContent=(med*u.f).toFixed(u.d);bigM.setAttribute("fill",rawColor(med/2));
   uL.textContent=u.s;uM.textContent=u.s;
   const full=LOAD_DANGER*2;
   heatL.setAttribute("opacity",Math.min(.45,lat/full*.8).toFixed(3));
   heatM.setAttribute("opacity",Math.min(.45,med/full*.8).toFixed(3));

   // center of load
   const tot=w[0]+w[1]+w[2]+w[3];let nx=0,ny=0;
   if(tot>0){nx=(-w[0]+w[1]-w[2]+w[3])/tot;ny=(-w[0]-w[1]+w[2]+w[3])/tot;}
   dot.style.left=(50+nx*40)+"%";dot.style.top=(50+ny*34)+"%";

   if(!paused){for(let i=0;i<4;i++){hist[i].push(w[i]);if(hist[i].length>HIST)hist[i].shift();}}
   drawChart();
  }

  function niceMax(v){if(v<=0)return 1;const p=Math.pow(10,Math.floor(Math.log10(v)));const n=v/p;
   return (n<=1?1:n<=2?2:n<=5?5:10)*p;}

  function drawChart(){
   const c=document.getElementById("chart"),ctx=c.getContext("2d");
   const dpr=window.devicePixelRatio||1,w=c.clientWidth,h=c.clientHeight;
   if(c.width!==Math.round(w*dpr)||c.height!==Math.round(h*dpr)){c.width=Math.round(w*dpr);c.height=Math.round(h*dpr);}
   ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,w,h);
   const cs=getComputedStyle(document.body);
   const cBorder=cs.getPropertyValue("--border").trim(),cMuted=cs.getPropertyValue("--muted").trim();
   const cols=[cs.getPropertyValue("--c0").trim(),cs.getPropertyValue("--c1").trim(),
               cs.getPropertyValue("--c2").trim(),cs.getPropertyValue("--c3").trim()];
   const u=UNITS[cur.metric];
   const pl=46,pr=6,pt=8,pb=16,pw=w-pl-pr,ph=h-pt-pb;

   let mx=0;for(let i=0;i<4;i++)for(const v of hist[i])if(v>mx)mx=v;
   const top=niceMax(Math.max(mx*1.15,LOAD_WARN*0.6));

   ctx.font="10px -apple-system,Segoe UI,Roboto,sans-serif";ctx.fillStyle=cMuted;ctx.textAlign="right";
   ctx.strokeStyle=cBorder;ctx.lineWidth=1;
   for(let g=0;g<=4;g++){
    const y=pt+ph-ph*g/4;
    ctx.globalAlpha=.6;ctx.beginPath();ctx.moveTo(pl,y+.5);ctx.lineTo(pl+pw,y+.5);ctx.stroke();ctx.globalAlpha=1;
    ctx.fillText((top*g/4*u.f).toFixed(u.d),pl-6,y+3.5);
   }

   [[LOAD_WARN,cs.getPropertyValue("--warn").trim()],[LOAD_DANGER,cs.getPropertyValue("--danger").trim()]].forEach(function(t){
    if(t[0]>top)return;const y=pt+ph-ph*t[0]/top;
    ctx.save();ctx.setLineDash([4,4]);ctx.strokeStyle=t[1];ctx.globalAlpha=.55;
    ctx.beginPath();ctx.moveTo(pl,y+.5);ctx.lineTo(pl+pw,y+.5);ctx.stroke();ctx.restore();
   });

   for(let i=0;i<4;i++){
    const a=hist[i];if(a.length<2)continue;
    ctx.beginPath();ctx.lineWidth=2;ctx.lineJoin="round";ctx.strokeStyle=cols[i];
    for(let j=0;j<a.length;j++){
     const x=pl+pw*(j+HIST-a.length)/(HIST-1);
     const y=pt+ph-ph*Math.min(a[j],top)/top;
     j?ctx.lineTo(x,y):ctx.moveTo(x,y);
    }
    ctx.stroke();
    const ly=pt+ph-ph*Math.min(a[a.length-1],top)/top;
    ctx.fillStyle=cols[i];ctx.beginPath();ctx.arc(pl+pw,ly,2.6,0,6.29);ctx.fill();
   }

   ctx.textAlign="left";ctx.fillStyle=cMuted;
   ctx.fillText("-"+((HIST*0.5)|0)+"s",pl+1,h-4);
   ctx.textAlign="right";ctx.fillText("now",pl+pw,h-4);
   ctx.textAlign="left";ctx.fillText(u.s,pl-40,pt+7);
  }

  function togglePause(){paused=!paused;const t=T[cur.lang];
   btnPause.textContent=paused?t.resume:t.pause;btnPause.classList.toggle("active",paused);}
  function clearHist(){hist=[[],[],[],[]];drawChart();}

  async function poll(){
   try{const d=await (await fetch("/data")).json();
    cur.theme=d.theme;cur.lang=d.lang;cur.metric=d.metric;cur.auto=d.auto;
    applyTheme();applyLang();render(d);}catch(e){}
  }
  function openPanel(){selTheme.value=cur.theme;selLang.value=cur.lang;selMetric.value=cur.metric;
   chkAuto.checked=cur.auto;selMat.value=mat;panel.classList.add("open");}
  function closePanel(){panel.classList.remove("open");}
  async function saveAndClose(){const q="theme="+selTheme.value+"&lang="+selLang.value+"&metric="+selMetric.value+"&auto="+(chkAuto.checked?1:0);
   await fetch("/set?"+q);panel.classList.remove("open");poll();}
  window.addEventListener("resize",drawChart);

applyMat(mat);setInterval(poll,500);poll();
</script>
</body></html>
)HTML";