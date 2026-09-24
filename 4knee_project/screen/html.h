#pragma once
const char PAGE[] = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Knee Monitor</title>
<style>
:root{--bg:#060b10;--surface:#0c151c;--surface2:#111e27;--border:#1c2c38;--text:#e3edf3;--muted:#7d93a3;--accent:#2dd4bf;--warn:#fbbf24;--danger:#f87171;--accent-soft:rgba(45,212,191,.14);--danger-soft:rgba(248,113,113,.14);--c0:#22d3ee;--c1:#facc15;--c2:#f472b6;--c3:#4ade80;
--grid:rgba(45,212,191,.05);--grid2:rgba(45,212,191,.13);--shadow:0 1px 2px rgba(0,0,0,.5);--radius:14px;
--b1:#fbf4e7;--b2:#eaddc4;--b3:#d5c0a0;--b4:#a98b60;--b5:#8a7047;--bink:#4a3a22;--gloss:.22;--mott:.38;}
body.light{--bg:#edf2f6;--surface:#fff;--surface2:#f5f8fb;--border:#d6e0e8;--text:#0f2537;--muted:#587084;--accent:#0e7c86;--warn:#b45309;--danger:#c81e1e;--accent-soft:rgba(14,124,134,.1);--danger-soft:rgba(200,30,30,.08);--c0:#0284c7;--c1:#ca8a04;--c2:#c026d3;--c3:#16a34a;
--grid:rgba(220,38,38,.06);--grid2:rgba(220,38,38,.15);--shadow:0 1px 2px rgba(15,37,55,.06),0 6px 16px rgba(15,37,55,.06);}
body.poly{--b1:#fffde8;--b2:#f6ee46;--b3:#ddd400;--b4:#9e9800;--b5:#6f6b00;--bink:#3a3800;--gloss:.55;--mott:.12;}
*{box-sizing:border-box;margin:0;padding:0;}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans Thai",sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:14px 14px 28px;transition:background .25s,color .25s;-webkit-tap-highlight-color:transparent;}
.wrap{max-width:520px;margin:0 auto;}

/* header */
.header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:10px;}
.brand{display:flex;align-items:center;gap:10px;min-width:0;}
.logo{width:40px;height:40px;flex:none;}
.logo rect{fill:var(--accent);}
.logo polyline{fill:none;stroke:#fff;stroke-width:2.4;stroke-linecap:round;stroke-linejoin:round;}
.header h1{font-size:clamp(1rem,4.3vw,1.2rem);font-weight:700;letter-spacing:-.01em;line-height:1.15;}
.sub{font-size:.7rem;color:var(--muted);margin-top:2px;letter-spacing:.2px;}
.hright{display:flex;align-items:center;gap:10px;flex:none;}
.clock{font-size:.8rem;font-weight:600;color:var(--muted);font-variant-numeric:tabular-nums;letter-spacing:.6px;}
.gear{flex:none;width:40px;height:40px;border-radius:12px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.15rem;cursor:pointer;box-shadow:var(--shadow);transition:.15s;}
.gear:active{transform:scale(.92);}
.statusbar{display:flex;align-items:center;gap:8px;margin-bottom:10px;}
.status{display:inline-flex;align-items:center;gap:7px;padding:5px 12px;border-radius:999px;font-size:.76rem;font-weight:600;letter-spacing:.2px;border:1px solid transparent;}
.status.on{background:var(--accent-soft);color:var(--accent);border-color:var(--accent-soft);}
.status.off{background:var(--danger-soft);color:var(--danger);border-color:var(--danger-soft);}
.status::before{content:"";width:8px;height:8px;border-radius:50%;background:currentColor;}
.status.on::before{animation:pulse 1.6s infinite;}
@keyframes pulse{0%,100%{opacity:1;box-shadow:0 0 0 0 currentColor}50%{opacity:.35}}

/* cards */
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow);}
.cardhead{display:flex;align-items:baseline;justify-content:space-between;gap:8px;padding:11px 13px 0;}
.ttl,.chead h3{font-size:.68rem;font-weight:700;letter-spacing:1.3px;text-transform:uppercase;color:var(--muted);}
.tagline{font-size:.66rem;color:var(--muted);opacity:.85;}

/* imaging viewport around the knee */
.vp{position:relative;margin:9px 10px 10px;padding:20px 14px 12px;border-radius:10px;background-color:var(--surface2);
 background-image:linear-gradient(var(--grid) 1px,transparent 1px),linear-gradient(90deg,var(--grid) 1px,transparent 1px);background-size:14px 14px;}
.vp::before{content:"";position:absolute;inset:6px;pointer-events:none;opacity:.7;
 background:linear-gradient(var(--accent),var(--accent)) left top/16px 2px no-repeat,linear-gradient(var(--accent),var(--accent)) left top/2px 16px no-repeat,
 linear-gradient(var(--accent),var(--accent)) right top/16px 2px no-repeat,linear-gradient(var(--accent),var(--accent)) right top/2px 16px no-repeat,
 linear-gradient(var(--accent),var(--accent)) left bottom/16px 2px no-repeat,linear-gradient(var(--accent),var(--accent)) left bottom/2px 16px no-repeat,
 linear-gradient(var(--accent),var(--accent)) right bottom/16px 2px no-repeat,linear-gradient(var(--accent),var(--accent)) right bottom/2px 16px no-repeat;}
.canvas{position:relative;width:min(86vw,430px);aspect-ratio:320/250;margin:0 auto;}
.canvas svg.knee{position:absolute;inset:0;width:100%;height:100%;overflow:visible;}
.ori{position:absolute;font-size:.56rem;letter-spacing:1.6px;color:var(--muted);font-weight:700;}
.ori-t{top:-14px;left:50%;transform:translateX(-50%);}
.ori-b{bottom:-12px;left:50%;transform:translateX(-50%);}
.ori-l{left:-10px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%) rotate(180deg);}
.ori-r{right:-10px;top:50%;writing-mode:vertical-rl;transform:translateY(-50%);}
.dot{position:absolute;width:22px;height:22px;left:50%;top:50%;transform:translate(-50%,-50%);transition:left .22s ease,top .22s ease;pointer-events:none;z-index:3;}
.dot::before{content:"";position:absolute;inset:3px;border-radius:50%;background:var(--accent);box-shadow:0 0 0 3px rgba(255,255,255,.85),0 0 16px 4px var(--accent);}
.dot::after{content:"";position:absolute;inset:-9px;border:1.5px dashed var(--accent);border-radius:50%;opacity:.7;}
.copnote{text-align:center;font-size:.6rem;color:var(--muted);letter-spacing:1.4px;font-weight:600;margin-top:14px;}
.copnote::before{content:"";display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--accent);margin-right:6px;vertical-align:1px;}

.big{font-weight:700;font-size:30px;font-variant-numeric:tabular-nums;paint-order:stroke;stroke:rgba(0,0,0,.45);stroke-width:3px;stroke-linejoin:round;}
.unit{font-size:10px;font-weight:600;letter-spacing:2px;fill:var(--bink);opacity:.75;}
.small{font-weight:700;font-size:13px;font-variant-numeric:tabular-nums;paint-order:stroke;stroke:rgba(0,0,0,.35);stroke-width:2.5px;stroke-linejoin:round;}
.tag{font-size:8.5px;font-weight:700;letter-spacing:1.4px;fill:var(--bink);opacity:.55;}
.etch{fill:none;stroke:var(--bink);opacity:.16;}
body.light .big,body.light .small{stroke:rgba(255,255,255,.9);}
 #specA,#specB,#specC{opacity:var(--gloss);}
 #mottle{opacity:var(--mott);}

/* trend chart */
.chartcard{margin-top:12px;padding:12px 12px 12px;}
.chead{display:flex;align-items:center;justify-content:space-between;margin-bottom:2px;}
.chead .live{font-size:.62rem;font-weight:700;letter-spacing:1px;color:var(--accent);display:flex;align-items:center;gap:5px;}
.chead .live::before{content:"";width:6px;height:6px;border-radius:50%;background:currentColor;animation:pulse 1.6s infinite;}
canvas{width:100%;height:180px;display:block;}
.rangebtns{display:flex;gap:2px;margin:8px 0 8px;padding:3px;background:var(--surface2);border:1px solid var(--border);border-radius:10px;}
.rangebtns.hide{display:none;}
.rbtn{flex:1;border:none;background:transparent;color:var(--muted);border-radius:7px;padding:6px 0;font-size:.72rem;font-weight:600;cursor:pointer;}
.rbtn.active{background:var(--surface);color:var(--accent);box-shadow:0 1px 3px rgba(0,0,0,.2);}

/* vital-sign tiles */
.legend{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;margin-top:10px;}
.lg{background:var(--surface2);border:1px solid var(--border);border-top:3px solid var(--lc);border-radius:8px;padding:6px 4px 6px;text-align:center;}
.lg b{display:block;font-size:.62rem;font-weight:800;letter-spacing:1.2px;color:var(--lc);}
.lg span{display:block;font-size:1.08rem;font-weight:700;color:var(--text);font-variant-numeric:tabular-nums;line-height:1.25;}
.lg em{display:block;font-style:normal;font-size:.56rem;color:var(--muted);letter-spacing:1px;}
.foot{text-align:center;font-size:.6rem;color:var(--muted);letter-spacing:.8px;margin-top:14px;opacity:.8;}

/* settings panel */
.panel{position:fixed;inset:0;background:var(--bg);z-index:10;display:flex;flex-direction:column;padding:20px 16px;transform:translateY(100%);transition:transform .28s cubic-bezier(.4,0,.2,1);}
.panel.open{transform:translateY(0);}
.phead{display:flex;align-items:center;justify-content:space-between;max-width:460px;width:100%;margin:0 auto 4px;}
.phead h2{font-size:1.15rem;font-weight:700;}
.close{width:38px;height:38px;border-radius:10px;border:1px solid var(--border);background:var(--surface);color:var(--text);font-size:1.1rem;cursor:pointer;}
.pbody{max-width:460px;width:100%;margin:0 auto;overflow:auto;}
.row{display:flex;align-items:center;justify-content:space-between;gap:16px;padding:14px 2px;border-bottom:1px solid var(--border);}
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
.dbtn{flex:1;padding:13px;border-radius:10px;border:1px solid var(--border);background:var(--surface2);color:var(--text);font-size:.85rem;font-weight:600;cursor:pointer;}
.dbtn.danger{border-color:var(--danger);color:var(--danger);}
.dbtn:active{transform:scale(.97);}
.seclabel{font-size:.7rem;color:var(--muted);letter-spacing:1.2px;text-transform:uppercase;font-weight:700;padding:18px 2px 2px;}
@media (prefers-reduced-motion:reduce){*{animation:none!important;transition:none!important;}}
</style></head><body>

<div class="wrap">
 <div class="header">
  <div class="brand">
   <svg class="logo" viewBox="0 0 40 40"><rect x="0" y="0" width="40" height="40" rx="11"/><polyline points="6,21 13,21 16,14 20,28 24,9 27,21 34,21"/></svg>
   <div><h1 id="title">Knee Load Monitor</h1><div class="sub" id="sub">4-channel tibial load sensor</div></div>
  </div>
  <div class="hright"><span class="clock" id="clock">--:--</span>
   <button class="gear" onclick="openPanel()">&#9881;</button></div>
 </div>
 <div class="statusbar"><div id="status" class="status off">...</div></div>

 <div class="card">
 <div class="cardhead"><span class="ttl" id="kTitle">Load distribution</span><span class="tagline" id="kSub">Tibial plateau &middot; top view</span></div>
 <div class="vp">
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

   <use href="#pOut" xlink:href="#pOut" fill="#000" opacity=".45" transform="translate(4,10)" filter="url(#fDrop)"/>
   <use href="#pOut" xlink:href="#pOut" fill="url(#gEdge)"/>
   <g transform="translate(160,124) scale(.965) translate(-160,-124)">
    <use href="#pOut" xlink:href="#pOut" fill="url(#gBody)"/>
   </g>

   <g clip-path="url(#clipBody)">
    <rect id="mottle" x="0" y="0" width="320" height="250" fill="#e6d3b4" filter="url(#fGrain)"/>

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

    <path d="M150 44 C143 78 143 120 152 148 C157 162 163 162 168 148 C177 120 177 78 170 44 Z"
          fill="url(#gEdge)" opacity=".9"/>
    <path d="M154 52 C148 84 148 120 156 146" fill="none" stroke="#fff" stroke-width="2.6" opacity=".38"/>
    <path d="M166 52 C172 84 172 120 164 146" fill="none" stroke="#000" stroke-width="2" opacity=".18"/>

    <ellipse id="specA" cx="86" cy="52" rx="46" ry="15" fill="url(#gSpec)" transform="rotate(-13 86 52)" filter="url(#fBlur3)"/>
    <ellipse id="specB" cx="238" cy="54" rx="42" ry="13" fill="url(#gSpec)" transform="rotate(11 238 54)" filter="url(#fBlur3)"/>
    <ellipse id="specC" cx="60" cy="150" rx="10" ry="52" fill="url(#gSpec)" transform="rotate(6 60 150)" filter="url(#fBlur6)"/>

    <rect x="0" y="0" width="320" height="250" fill="url(#gVig)"/>
   </g>

   <use href="#pOut" xlink:href="#pOut" fill="none" stroke="#8a7047" style="stroke:var(--b5)" stroke-width="2.2"/>

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
 </div>
 </div>

 <div class="chartcard card">
  <div class="chead">
   <h3 id="chTitle">Load over time</h3><span class="live" id="liveTag">LIVE</span>
  </div>
  <div class="rangebtns" id="rangeBtns">
   <button class="rbtn active" data-r="0" onclick="setView(0)">Live</button>
   <button class="rbtn" data-r="1" onclick="setView(1)">1W</button>
   <button class="rbtn" data-r="2" onclick="setView(2)">1M</button>
   <button class="rbtn" data-r="3" onclick="setView(3)">1Y</button>
  </div>
  <canvas id="chart"></canvas>
  <div class="legend">
   <div class="lg" style="--lc:var(--c0)"><b>AL</b><span id="lv0">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c1)"><b>AM</b><span id="lv1">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c2)"><b>PL</b><span id="lv2">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c3)"><b>PM</b><span id="lv3">0</span><em class="lu">g</em></div>
  </div>
 </div>
 <div class="foot" id="foot">KNEE LOAD MONITOR &middot; ESP-NOW 4-CH</div>
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
  <div class="row"><span class="rlabel" id="lblGType">Graph type<small id="lblGTypeSub">Bar graph shows live data only</small></span>
   <select id="selGType"><option value="0">Line</option><option value="1">Bar</option></select></div>
  <div class="row"><span class="rlabel" id="lblMat">Material<small id="lblMatSub">Stored on this phone only</small></span>
   <select id="selMat" onchange="applyMat(this.value)"><option value="0">Bone</option><option value="1">Trial insert</option></select></div>

  <div class="seclabel" id="lblData">Data</div>
  <div class="row"><span class="rlabel" id="lblKeep">Auto-clear<small id="lblKeepSub">Old data is removed automatically</small></span>
   <select id="selKeep" onchange="setKeep(this.value)">
    <option value="0">Off</option>
    <option value="1">1 week</option>
    <option value="2">1 month</option>
    <option value="3">1 year</option>
   </select></div>
  <div class="row" style="border-bottom:none;gap:10px">
   <button class="dbtn" id="btnDownload" onclick="downloadCsv()">Download CSV</button>
   <button class="dbtn danger" id="btnDelete" onclick="deleteCsv()">Delete all</button>
  </div>

  <button class="save" id="btnSave" onclick="saveAndClose()">Save &amp; Close</button>
 </div>
</div>

<script>
const LOAD_WARN=20000, LOAD_DANGER=35000;
const T={
 0:{title:"จอวัดน้ำหนักลงเข่า",sub:"เซ็นเซอร์วัดแรงกด 4 จุด",kTitle:"การกระจายแรงกด",kSub:"ผิวกระดูกหน้าแข้ง · มุมบน",live:"สด",settings:"ตั้งค่า",theme:"ธีม",auto:"ธีมอัตโนมัติ",lang:"ภาษา",unit:"หน่วย",save:"บันทึกและปิด",on:"เชื่อมต่อแล้ว",off:"ขาดการเชื่อมต่อ",ant:"ด้านหน้า",post:"ด้านหลัง",med:"ด้านใน",lat:"ด้านนอก",cop:"จุดศูนย์กลางแรงกด",chart:"กราฟแรงกดตามเวลา",gtype:"รูปแบบกราฟ",gtypesub:"กราฟแท่งแสดงเฉพาะข้อมูลสด",mat:"พื้นผิว",matsub:"บันทึกในเครื่องนี้เท่านั้น",m0:"กระดูก",m1:"แผ่นทดลอง",line:"เส้น",bar:"แท่ง",nosync:"ยังไม่ได้ซิงค์เวลา",nodata:"ไม่มีข้อมูล",data:"ข้อมูล",keep:"ล้างอัตโนมัติ",keepsub:"ลบข้อมูลเก่าโดยอัตโนมัติ",dl:"ดาวน์โหลด CSV",del:"ลบทั้งหมด",confirmDel:"ลบข้อมูลทั้งหมด?"},
 1:{title:"Knee Load Monitor",sub:"4-channel tibial load sensor",kTitle:"Load distribution",kSub:"Tibial plateau · top view",live:"LIVE",settings:"Settings",theme:"Theme",auto:"Auto theme",lang:"Language",unit:"Unit",save:"Save & Close",on:"Connected",off:"Disconnected",ant:"ANTERIOR",post:"POSTERIOR",med:"MEDIAL",lat:"LATERAL",cop:"CENTER OF LOAD",chart:"Load over time",gtype:"Graph type",gtypesub:"Bar graph shows live data only",mat:"Material",matsub:"Stored on this phone only",m0:"Bone",m1:"Trial insert",line:"Line",bar:"Bar",nosync:"No time sync yet",nodata:"No data",data:"Data",keep:"Auto-clear",keepsub:"Old data is removed automatically",dl:"Download CSV",del:"Delete all",confirmDel:"Delete all data?"}
};
const UNITS=[{f:1,s:"g",d:0},{f:0.00980665,s:"N",d:1},{f:0.001,s:"kg",d:2}];
let cur={theme:1,lang:1,metric:0,auto:true,gtype:0,keep:0};

  const HIST=120;
  let hist=[[],[],[],[]], mat=0;
  let view=0;
  let histView=[[],[],[],[]];
  let histMsg="";
  const statusEl=document.getElementById("status");
  let lastLang=-1, busy=false;

  try{mat=parseInt(localStorage.getItem("kneeMat")||"0",10)||0;}catch(e){}

  function applyMat(v){mat=parseInt(v,10)||0;document.body.classList.toggle("poly",mat===1);
   try{localStorage.setItem("kneeMat",mat);}catch(e){}}
  function applyTheme(){let t=cur.theme;if(cur.auto){const h=new Date().getHours();t=(h>=7&&h<19)?0:1;}
   document.body.classList.toggle("light",t===0);document.body.classList.toggle("poly",mat===1);}
  function applyLang(){if(cur.lang===lastLang)return;lastLang=cur.lang;const t=T[cur.lang];
   title.textContent=t.title;setTitle.textContent=t.settings;lblTheme.textContent=t.theme;lblAuto.textContent=t.auto;
   lblLang.textContent=t.lang;lblUnit.textContent=t.unit;btnSave.textContent=t.save;
   lblMat.firstChild.nodeValue=t.mat;lblMatSub.textContent=t.matsub;
   selMat.options[0].text=t.m0;selMat.options[1].text=t.m1;
   oriT.textContent=t.ant;oriB.textContent=t.post;oriL.textContent=t.lat;oriR.textContent=t.med;
   copnote.textContent=t.cop;chTitle.textContent=t.chart;
   sub.textContent=t.sub;kTitle.textContent=t.kTitle;kSub.textContent=t.kSub;liveTag.textContent=t.live;
   lblGType.firstChild.nodeValue=t.gtype;lblGTypeSub.textContent=t.gtypesub;
   lblData.textContent=t.data;lblKeep.firstChild.nodeValue=t.keep;lblKeepSub.textContent=t.keepsub;
   btnDownload.textContent=t.dl;btnDelete.textContent=t.del;}
  function applyType(){const t=T[cur.lang];
   selGType.options[0].text=t.line;selGType.options[1].text=t.bar;
   const bar=(cur.gtype===1);
   rangeBtns.classList.toggle("hide",bar);
   if(bar&&view!==0)setView(0);}

  function rawColor(v){const cs=getComputedStyle(document.body);
   return cs.getPropertyValue(v>=LOAD_DANGER?"--danger":(v>=LOAD_WARN?"--warn":"--accent")).trim();}

  function setView(v){view=v;liveTag.style.visibility=(v===0)?"visible":"hidden";if(v===0)liveTag.textContent=T[cur.lang].live;
   document.querySelectorAll(".rbtn").forEach(function(b){b.classList.toggle("active",parseInt(b.dataset.r,10)===v);});
   histMsg="";yLo=null;
   if(v===0){drawChart();}else{fetchHistory();}}

  async function fetchHistory(){
   const map={1:"w",2:"m",3:"y"};
   try{const d=await (await fetch("/history?r="+map[view])).json();
    if(d.err){histMsg=d.err==="notime"?T[cur.lang].nosync:T[cur.lang].nodata;histView=[[],[],[],[]];drawChart();return;}
    histView=d.k;
    let any=false;for(let i=0;i<4;i++)for(const v of histView[i])if(v>=0){any=true;break;}
    histMsg=any?"":T[cur.lang].nodata;
    drawChart();
   }catch(e){histMsg=T[cur.lang].nodata;drawChart();}}

  function render(d){
   const t=T[cur.lang],u=UNITS[cur.metric];
   statusEl.textContent=d.connected?t.on:t.off;
   statusEl.className="status "+(d.connected?"on":"off");
   const w=d.k;
   for(let i=0;i<4;i++){
    const c=rawColor(w[i]),txt=(w[i]*u.f).toFixed(u.d);
    const s=document.getElementById("s"+i);s.textContent=txt;s.setAttribute("fill",c);
    document.getElementById("p"+i).setAttribute("fill",c);
    document.getElementById("lv"+i).textContent=txt;
   }
   const lat=w[0]+w[2], med=w[1]+w[3];
   bigL.textContent=(lat*u.f).toFixed(u.d);bigL.setAttribute("fill",rawColor(lat/2));
   bigM.textContent=(med*u.f).toFixed(u.d);bigM.setAttribute("fill",rawColor(med/2));
   uL.textContent=u.s;uM.textContent=u.s;
   document.querySelectorAll(".lu").forEach(function(e){e.textContent=u.s;});
   const full=LOAD_DANGER*2;
   heatL.setAttribute("opacity",Math.min(.45,lat/full*.8).toFixed(3));
   heatM.setAttribute("opacity",Math.min(.45,med/full*.8).toFixed(3));
   const tot=w[0]+w[1]+w[2]+w[3];let nx=0,ny=0;
   if(tot>0){nx=(-w[0]+w[1]-w[2]+w[3])/tot;ny=(-w[0]-w[1]+w[2]+w[3])/tot;}
   dot.style.left=(50+nx*40)+"%";dot.style.top=(50+ny*34)+"%";
   for(let i=0;i<4;i++){hist[i].push(w[i]);if(hist[i].length>HIST)hist[i].shift();}
   if(view===0)drawChart();
  }

  /* ---------- dynamic Y scale: follows the incoming data ---------- */
  let yLo=null,yHi=null,raf=0;
  const reduceMotion=!!(window.matchMedia&&matchMedia("(prefers-reduced-motion: reduce)").matches);

  // "nice" step (1, 2, 2.5, 5 x 10^n) that splits span into ~n parts
  function niceStep(span,n){if(!(span>0))return 1;const raw=span/n,p=Math.pow(10,Math.floor(Math.log10(raw))),f=raw/p;
   return (f<=1?1:f<=2?2:f<=2.5?2.5:f<=5?5:10)*p;}
  function decOf(s){for(let d=0;d<6;d++){const x=s*Math.pow(10,d);if(Math.abs(x-Math.round(x))<1e-6)return d;}return 6;}

  // target range (raw grams) computed from what is on screen right now
  function targetRange(data,live,bars,uf){
   let mn=Infinity,mx=-Infinity;
   for(let i=0;i<4;i++){const a=data[i];
    if(bars){for(let k=a.length-1;k>=0;k--){if(live||a[k]>=0){if(a[k]<mn)mn=a[k];if(a[k]>mx)mx=a[k];break;}}}
    else{for(const v of a){if(!live&&v<0)continue;if(v<mn)mn=v;if(v>mx)mx=v;}}
   }
   if(mn===Infinity){mn=0;mx=1000;}                 // no data yet
   const dataMin=mn;
   if(bars&&mn>0)mn=0;                               // bars always grow from 0
   let span=mx-mn;
   const minSpan=Math.max(Math.abs(mx)*0.05,10);     // don't zoom into sensor noise (<10 g)
   if(span<minSpan){const c=(mx+mn)/2;mn=c-minSpan/2;mx=c+minSpan/2;span=minSpan;}
   let lo=bars?mn:mn-span*0.08, hi=mx+span*0.10;     // headroom
   if(lo<0&&dataMin>=0)lo=0;                         // no negative axis for positive data
   const st=niceStep((hi-lo)*uf,4);                  // round to nice numbers in display unit
   return {lo:Math.floor(lo*uf/st)*st/uf, hi:Math.ceil(hi*uf/st)*st/uf};
  }

  function drawChart(){
   if(raf){cancelAnimationFrame(raf);raf=0;}
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
   const g1=cs.getPropertyValue("--grid").trim(),g2=cs.getPropertyValue("--grid2").trim();
   ctx.lineWidth=1;
   for(let x=0;x<=pw;x+=10){ctx.strokeStyle=(x%50===0)?g2:g1;ctx.beginPath();ctx.moveTo(pl+x+.5,pt);ctx.lineTo(pl+x+.5,pt+ph);ctx.stroke();}
   for(let y=0;y<=ph;y+=10){ctx.strokeStyle=(y%50===0)?g2:g1;ctx.beginPath();ctx.moveTo(pl,pt+ph-y+.5);ctx.lineTo(pl+pw,pt+ph-y+.5);ctx.stroke();}
   const glow=!document.body.classList.contains("light");

   const live=(view===0);
   const data=live?hist:histView;
   const NP=live?HIST:120;
   const bars=(cur.gtype===1);

   if(!live && histMsg){
    yLo=null;
    ctx.fillStyle=cMuted;ctx.font="12px -apple-system,Segoe UI,Roboto,sans-serif";
    ctx.textAlign="center";ctx.fillText(histMsg,w/2,h/2);return;}

   // ease the axis toward the target so it doesn't jump every sample
   const tg=targetRange(data,live,bars,u.f);
   if(yLo===null||reduceMotion){yLo=tg.lo;yHi=tg.hi;}
   else{
    yLo+=(tg.lo-yLo)*0.2;yHi+=(tg.hi-yHi)*0.2;
    if(Math.abs(tg.lo-yLo)+Math.abs(tg.hi-yHi)<(tg.hi-tg.lo)*0.003){yLo=tg.lo;yHi=tg.hi;}
    else raf=requestAnimationFrame(drawChart);
   }
   const span=(yHi-yLo)||1;
   function yOf(v){return pt+ph-ph*(Math.max(yLo,Math.min(yHi,v))-yLo)/span;}

   // grid + labels (nice numbers in the selected unit)
   ctx.font="10px -apple-system,Segoe UI,Roboto,sans-serif";ctx.fillStyle=cMuted;ctx.textAlign="right";
   ctx.strokeStyle=cBorder;ctx.lineWidth=1;
   const stepD=niceStep(span*u.f,4),dec=decOf(stepD);
   for(let k=Math.ceil(yLo*u.f/stepD-1e-9);k*stepD<=yHi*u.f+1e-9;k++){
    const y=yOf(k*stepD/u.f);
    ctx.globalAlpha=.6;ctx.beginPath();ctx.moveTo(pl,y+.5);ctx.lineTo(pl+pw,y+.5);ctx.stroke();ctx.globalAlpha=1;
    ctx.fillText((k*stepD).toFixed(dec),pl-6,y+3.5);
   }
   [[LOAD_WARN,cs.getPropertyValue("--warn").trim()],[LOAD_DANGER,cs.getPropertyValue("--danger").trim()]].forEach(function(tr){
    if(tr[0]<yLo||tr[0]>yHi)return;const y=yOf(tr[0]);
    ctx.save();ctx.setLineDash([4,4]);ctx.strokeStyle=tr[1];ctx.globalAlpha=.55;
    ctx.beginPath();ctx.moveTo(pl,y+.5);ctx.lineTo(pl+pw,y+.5);ctx.stroke();ctx.restore();
   });

   function xAt(j,len){return live?pl+pw*(j+HIST-len)/(HIST-1):pl+pw*j/(NP-1);}

   if(bars){
    const gap=12,bw=(pw-gap*5)/4,names=["AL","AM","PL","PM"],base=yOf(0);
    ctx.textAlign="center";
    for(let i=0;i<4;i++){
     const a=data[i];let v=0;for(let k=a.length-1;k>=0;k--){if(live||a[k]>=0){v=a[k];break;}}
     const y=yOf(v),x=pl+gap+i*(bw+gap);
     ctx.fillStyle=cols[i];ctx.fillRect(x,Math.min(y,base),bw,Math.max(Math.abs(base-y),1));
     ctx.fillStyle=cMuted;ctx.fillText(names[i],x+bw/2,h-4);
    }
   }else{
    for(let i=0;i<4;i++){
     const a=data[i];if(a.length<2)continue;
     ctx.lineWidth=2;ctx.lineJoin="round";ctx.strokeStyle=cols[i];ctx.shadowColor=cols[i];ctx.shadowBlur=glow?7:0;ctx.beginPath();
     let pen=false,lx=0,ly=0;
     for(let j=0;j<a.length;j++){
      if(!live&&a[j]<0){pen=false;continue;}           // -1 = empty bucket in history
      const x=xAt(j,a.length),y=yOf(a[j]);
      pen?ctx.lineTo(x,y):ctx.moveTo(x,y);pen=true;lx=x;ly=y;
     }
     ctx.stroke();ctx.shadowBlur=0;
     if(pen){ctx.fillStyle=cols[i];ctx.beginPath();ctx.arc(lx,ly,2.6,0,6.29);ctx.fill();}
    }
    ctx.fillStyle=cMuted;ctx.textAlign="left";
    const lbl=live?("-"+((HIST*0.5)|0)+"s"):(view===1?"-7d":view===2?"-30d":"-1y");
    ctx.fillText(lbl,pl+1,h-4);
    ctx.textAlign="right";ctx.fillText("now",pl+pw,h-4);
   }
   ctx.textAlign="left";ctx.fillStyle=cMuted;ctx.fillText(u.s,4,h-4);
  }

  function clearHist(){if(view===0){hist=[[],[],[],[]];}yLo=null;drawChart();}

  function downloadCsv(){window.location.href="/download";}
  function deleteCsv(){if(confirm(T[cur.lang].confirmDel)){fetch("/clear").then(function(){clearHist();});}}
  function setKeep(v){cur.keep=parseInt(v,10)||0;fetch("/set?keep="+cur.keep).catch(function(){});}

  async function poll(){
   if(busy)return;busy=true;
   try{const d=await (await fetch("/data")).json();
    cur.theme=d.theme;cur.lang=d.lang;cur.metric=d.metric;cur.auto=d.auto;cur.gtype=d.graphtype;cur.keep=d.keep;
    applyTheme();applyLang();applyType();render(d);}catch(e){}
   busy=false;
  }
  function openPanel(){selTheme.value=cur.theme;selLang.value=cur.lang;selMetric.value=cur.metric;selGType.value=cur.gtype;
   chkAuto.checked=cur.auto;selMat.value=mat;selKeep.value=cur.keep;panel.classList.add("open");}
  function closePanel(){panel.classList.remove("open");}
  async function saveAndClose(){const q="theme="+selTheme.value+"&lang="+selLang.value+"&metric="+selMetric.value+"&graphtype="+selGType.value+"&auto="+(chkAuto.checked?1:0);
   await fetch("/set?"+q);panel.classList.remove("open");poll();}
  window.addEventListener("resize",drawChart);

  fetch("/time?t="+Math.floor(Date.now()/1000)).catch(function(){});
function tick(){const d=new Date(),p=function(n){return (n<10?"0":"")+n;};clock.textContent=p(d.getHours())+":"+p(d.getMinutes())+":"+p(d.getSeconds());}
applyMat(mat);applyType();setInterval(poll,500);poll();tick();setInterval(tick,1000);
</script>
</body></html>
)HTML";