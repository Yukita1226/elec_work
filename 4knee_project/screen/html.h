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
.main{display:flex;flex-direction:column;gap:12px;}

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
#p0{fill:var(--c0);}#p1{fill:var(--c1);}#p2{fill:var(--c2);}#p3{fill:var(--c3);}
#pads circle{transition:opacity .22s ease;}

/* trend chart */
.chartcard{padding:12px;display:flex;flex-direction:column;}
.chead{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:2px;}
.chright{display:flex;align-items:center;gap:10px;flex:none;}
.chead .live{font-size:.62rem;font-weight:700;letter-spacing:1px;color:var(--accent);display:flex;align-items:center;gap:5px;}
.chead .live::before{content:"";width:6px;height:6px;border-radius:50%;background:currentColor;animation:pulse 1.6s infinite;}
.gtog{display:flex;gap:2px;padding:3px;background:var(--surface2);border:1px solid var(--border);border-radius:10px;}
.gbtn{width:34px;height:28px;display:flex;align-items:center;justify-content:center;border:none;border-radius:7px;background:transparent;color:var(--muted);cursor:pointer;transition:.15s;}
.gbtn svg{width:17px;height:17px;}
.gbtn polyline{fill:none;stroke:currentColor;stroke-width:2.4;stroke-linecap:round;stroke-linejoin:round;}
.gbtn rect{fill:currentColor;}
.gbtn.active{background:var(--surface);color:var(--accent);box-shadow:0 1px 3px rgba(0,0,0,.2);}
.gbtn:active{transform:scale(.92);}
.plot{position:relative;height:180px;user-select:none;-webkit-user-select:none;}
#chart{position:absolute;inset:0;width:100%;height:100%;display:block;}
.plot.lz #chart{touch-action:pan-y;cursor:grab;}
.plot.lz.drag #chart{cursor:grabbing;}
.zreset{position:absolute;top:4px;right:6px;z-index:2;display:none;align-items:center;padding:3px 9px;border-radius:8px;border:1px solid var(--border);background:var(--surface);color:var(--accent);font-size:.68rem;font-weight:700;font-variant-numeric:tabular-nums;cursor:pointer;box-shadow:0 1px 3px rgba(0,0,0,.2);}
.zreset.show{display:inline-flex;}
.zreset:active{transform:scale(.94);}
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

@media (orientation:landscape) and (min-width:640px){
 body{padding:12px 16px 16px;}
 .wrap{max-width:1180px;}
 .main{display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1.2fr);align-items:stretch;}
 .kneecard{display:flex;flex-direction:column;}
 .kneecard .vp{flex:1;display:flex;flex-direction:column;justify-content:center;}
 .canvas{width:min(100%,460px,max(240px,calc((100vh - 210px) * 1.28)));}
 .plot{flex:1;height:auto;min-height:140px;}
}
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

 <div class="main">
 <div class="card kneecard">
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
    </g>
    <ellipse id="heatL" cx="92" cy="122" rx="52" ry="62" fill="#f85149" opacity="0"/>
    <ellipse cx="92" cy="122" rx="52" ry="62" fill="none" stroke="#fff" stroke-width="2" opacity=".22"/>

    <ellipse cx="228" cy="122" rx="52" ry="62" fill="url(#gDish)"/>
    <g clip-path="url(#clipDishM)">
     <ellipse cx="228" cy="122" rx="52" ry="62" fill="none" stroke="#6d5837" stroke-width="14" opacity=".45" filter="url(#fBlur6)"/>
     <ellipse cx="232" cy="150" rx="34" ry="26" fill="#fff" opacity=".16" filter="url(#fBlur6)"/>
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
    <text class="tag" x="70"  y="50">AL</text>
    <text class="tag" x="250" y="50">AM</text>
    <text class="tag" x="70"  y="206">PL</text>
    <text class="tag" x="250" y="206">PM</text>
    <text class="small" id="s0" x="104" y="50"  fill="#34d399">0</text>
    <text class="small" id="s1" x="216" y="50"  fill="#34d399">0</text>
    <text class="small" id="s2" x="104" y="206" fill="#34d399">0</text>
    <text class="small" id="s3" x="216" y="206" fill="#34d399">0</text>
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
   <h3 id="chTitle">Load over time</h3>
   <div class="chright">
    <span class="live" id="liveTag">LIVE</span>
    <div class="gtog" role="group">
     <button class="gbtn active" id="gLine" onclick="setGType(0)" title="Line" aria-label="Line"><svg viewBox="0 0 24 24"><polyline points="3,17 8,11 13,14 21,5"/></svg></button>
     <button class="gbtn" id="gBar" onclick="setGType(1)" title="Bar" aria-label="Bar"><svg viewBox="0 0 24 24"><rect x="3" y="12" width="4.5" height="9" rx="1"/><rect x="9.75" y="5" width="4.5" height="16" rx="1"/><rect x="16.5" y="9" width="4.5" height="12" rx="1"/></svg></button>
    </div>
   </div>
  </div>
  <div class="rangebtns" id="rangeBtns">
   <button class="rbtn active" data-r="0" id="rLive" onclick="setView(0)">Real time</button>
   <button class="rbtn" data-r="1" id="r1s" onclick="setView(1)">1 sec</button>
   <button class="rbtn" data-r="2" id="r5s" onclick="setView(2)">5 sec</button>
   <button class="rbtn" data-r="3" id="r10s" onclick="setView(3)">10 sec</button>
  </div>
  <div class="plot" id="plot"><canvas id="chart"></canvas><button class="zreset" id="zReset" onclick="resetZoom(true)" title="Reset zoom" aria-label="Reset zoom"></button></div>
  <div class="legend">
   <div class="lg" style="--lc:var(--c0)"><b>AL</b><span id="lv0">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c1)"><b>AM</b><span id="lv1">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c2)"><b>PL</b><span id="lv2">0</span><em class="lu">g</em></div>
   <div class="lg" style="--lc:var(--c3)"><b>PM</b><span id="lv3">0</span><em class="lu">g</em></div>
  </div>
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
 0:{title:"จอวัดน้ำหนักลงเข่า",sub:"เซ็นเซอร์วัดแรงกด 4 จุด",kTitle:"การกระจายแรงกด",kSub:"ผิวกระดูกหน้าแข้ง · มุมบน",live:"สด",settings:"ตั้งค่า",theme:"ธีม",auto:"ธีมอัตโนมัติ",lang:"ภาษา",unit:"หน่วย",save:"บันทึกและปิด",on:"เชื่อมต่อแล้ว",off:"ขาดการเชื่อมต่อ",ant:"ด้านหน้า",post:"ด้านหลัง",med:"ด้านใน",lat:"ด้านนอก",cop:"จุดศูนย์กลางแรงกด",chart:"กราฟแรงกดตามเวลา",gtype:"รูปแบบกราฟ",gtypesub:"กราฟแท่งแสดงเฉพาะข้อมูลสด",mat:"พื้นผิว",matsub:"บันทึกในเครื่องนี้เท่านั้น",m0:"กระดูก",m1:"แผ่นทดลอง",line:"เส้น",bar:"แท่ง",nosync:"ยังไม่ได้ซิงค์เวลา",nodata:"ไม่มีข้อมูล",data:"ข้อมูล",keep:"ล้างอัตโนมัติ",keepsub:"ลบข้อมูลเก่าโดยอัตโนมัติ",dl:"ดาวน์โหลด CSV",del:"ลบทั้งหมด",confirmDel:"ลบข้อมูลทั้งหมด?",rlive:"เรียลไทม์",r1s:"1 วินาที",r5s:"5 วินาที",r10s:"10 วินาที",loading:"กำลังโหลด…",rz:"รีเซ็ตการซูม"},
 1:{title:"Knee Load Monitor",sub:"4-channel tibial load sensor",kTitle:"Load distribution",kSub:"Tibial plateau · top view",live:"LIVE",settings:"Settings",theme:"Theme",auto:"Auto theme",lang:"Language",unit:"Unit",save:"Save & Close",on:"Connected",off:"Disconnected",ant:"ANTERIOR",post:"POSTERIOR",med:"MEDIAL",lat:"LATERAL",cop:"CENTER OF LOAD",chart:"Load over time",gtype:"Graph type",gtypesub:"Bar graph shows live data only",mat:"Material",matsub:"Stored on this phone only",m0:"Bone",m1:"Trial insert",line:"Line",bar:"Bar",nosync:"No time sync yet",nodata:"No data",data:"Data",keep:"Auto-clear",keepsub:"Old data is removed automatically",dl:"Download CSV",del:"Delete all",confirmDel:"Delete all data?",rlive:"Real time",r1s:"1 sec",r5s:"5 sec",r10s:"10 sec",loading:"Loading…",rz:"Reset zoom"}
};
const UNITS=[{f:1,s:"g",d:0},{f:0.00980665,s:"N",d:1},{f:0.001,s:"kg",d:2}];
let cur={theme:1,lang:1,metric:0,auto:true,gtype:0,keep:0};

  const POLL_MS=1000;
  const HIST=120;
  const HKEY=["","1","5","10"],HLBL=["","-2m","-10m","-20m"];
  const HDUR=[0,120,600,1200];          // seconds covered by each history view
  const PADXY=[[92,84],[228,84],[92,172],[228,172]];
  let hist=[[],[],[],[]], histT=[], mat=0;
  let view=0;
  let histView=[[],[],[],[]];
  let histMsg="";
  let histReq=0, histPending=false, histTimer=0;
  let gLock=0;
  // line-chart zoom state: visible window [z0,z1] as a fraction of the full series
  let z0=0,z1=1,lastGT=-1,gest=null,multi=false,lastTap=0,lastTapX=0;
  const ptrs=new Map();
  const PL=46,PR=6;                                 // chart left/right padding (px)
  const cv=document.getElementById("chart"),plotEl=document.getElementById("plot"),zBtn=document.getElementById("zReset");
  const statusEl=document.getElementById("status");
  let lastLang=-1, busy=false;

  try{mat=parseInt(localStorage.getItem("kneeMat")||"0",10)||0;}catch(e){}

  async function getJson(u){const c=new AbortController(),t=setTimeout(function(){c.abort();},4000);
   try{return await (await fetch(u,{signal:c.signal})).json();}finally{clearTimeout(t);}}

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
   btnDownload.textContent=t.dl;btnDelete.textContent=t.del;
   rLive.textContent=t.rlive;r1s.textContent=t.r1s;r5s.textContent=t.r5s;r10s.textContent=t.r10s;
   gLine.title=t.line;gLine.setAttribute("aria-label",t.line);
   gBar.title=t.bar;gBar.setAttribute("aria-label",t.bar);
   zBtn.title=t.rz;zBtn.setAttribute("aria-label",t.rz);}
  function applyType(){const t=T[cur.lang];
   selGType.options[0].text=t.line;selGType.options[1].text=t.bar;
   const bar=(cur.gtype===1);
   if(cur.gtype!==lastGT){lastGT=cur.gtype;z0=0;z1=1;ptrs.clear();gest=null;multi=false;plotEl.classList.remove("drag");}
   gLine.classList.toggle("active",!bar);gBar.classList.toggle("active",bar);
   rangeBtns.classList.toggle("hide",bar);
   plotEl.classList.toggle("lz",!bar);zoomUi();
   if(bar&&view!==0)setView(0);}

  function setGType(v){if(cur.gtype===v)return;
   cur.gtype=v;gLock=Date.now();yLo=null;
   applyType();drawChart();
   fetch("/set?graphtype="+v).catch(function(){});}

  function rawColor(v){const cs=getComputedStyle(document.body);
   return cs.getPropertyValue(v>=LOAD_DANGER?"--danger":(v>=LOAD_WARN?"--warn":"--accent")).trim();}

  function setView(v){view=v;z0=0;z1=1;zoomUi();liveTag.style.visibility=(v===0)?"visible":"hidden";if(v===0)liveTag.textContent=T[cur.lang].live;
   document.querySelectorAll(".rbtn").forEach(function(b){b.classList.toggle("active",parseInt(b.dataset.r,10)===v);});
   if(histTimer){clearInterval(histTimer);histTimer=0;}
   histMsg="";yLo=null;
   if(v===0){drawChart();}
   else{
    histView=[[],[],[],[]];histMsg=T[cur.lang].loading;drawChart();
    fetchHistory(true);
    histTimer=setInterval(fetchHistory,POLL_MS);
   }}

  async function fetchHistory(force){
   if(view===0||(histPending&&force!==true))return;
   const id=++histReq,v=view;histPending=true;
   try{const d=await getJson("/history?r="+HKEY[v]);
    if(id!==histReq||v!==view)return;
    if(d.err){histMsg=d.err==="notime"?T[cur.lang].nosync:T[cur.lang].nodata;histView=[[],[],[],[]];drawChart();return;}
    histView=d.k;
    let any=false;for(let i=0;i<4;i++)for(const x of histView[i])if(x>=0){any=true;break;}
    histMsg=any?"":T[cur.lang].nodata;
    drawChart();
   }catch(e){if(id===histReq&&v===view){histMsg=T[cur.lang].nodata;drawChart();}}
   finally{if(id===histReq)histPending=false;}}

  function render(d){
   const t=T[cur.lang],u=UNITS[cur.metric];
   statusEl.textContent=d.connected?t.on:t.off;
   statusEl.className="status "+(d.connected?"on":"off");
   const w=d.k;
   for(let i=0;i<4;i++){
    const c=rawColor(w[i]),txt=(w[i]*u.f).toFixed(u.d);
    const s=document.getElementById("s"+i);s.textContent=txt;s.setAttribute("fill",c);
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
    const MIN_TOT=4, RAMP=6; 
    const tot=w[0]+w[1]+w[2]+w[3]; let nx=0, ny=0;
    if(tot>MIN_TOT){
      const s=Math.min(1,(tot-MIN_TOT)/RAMP);      
      nx=s*(-w[0]+w[1]-w[2]+w[3])/tot;
      ny=s*(-w[0]-w[1]+w[2]+w[3])/tot;
    }
    dot.style.left=((160+68*nx)/320*100)+"%";
    dot.style.top =((128+44*ny)/250*100)+"%";
    dot.style.opacity=tot>MIN_TOT?1:.35; 
   const cx=Math.min(228,Math.max(92,160+128*nx)),cy=Math.min(172,Math.max(84,125+85*ny));
   for(let i=0;i<4;i++)document.getElementById("p"+i).style.opacity=Math.max(.15,1-Math.hypot(cx-PADXY[i][0],cy-PADXY[i][1])/140).toFixed(2);
   for(let i=0;i<4;i++){hist[i].push(w[i]);if(hist[i].length>HIST)hist[i].shift();}
   histT.push(Date.now());if(histT.length>HIST)histT.shift();
   if(view===0)drawChart();
  }

  /* ---------- dynamic Y scale: follows the incoming data ---------- */
  let yLo=null,yHi=null,raf=0;
  const reduceMotion=!!(window.matchMedia&&matchMedia("(prefers-reduced-motion: reduce)").matches);

  // "nice" step (1, 2, 2.5, 5 x 10^n) that splits span into ~n parts
  function niceStep(span,n){if(!(span>0))return 1;const raw=span/n,p=Math.pow(10,Math.floor(Math.log10(raw))),f=raw/p;
   return (f<=1?1:f<=2?2:f<=2.5?2.5:f<=5?5:10)*p;}
  function decOf(s){for(let d=0;d<6;d++){const x=s*Math.pow(10,d);if(Math.abs(x-Math.round(x))<1e-6)return d;}return 6;}

  // target range (raw grams) computed from what is on screen right now (line chart: only the zoomed window)
  function targetRange(data,live,bars,uf){
   let mn=Infinity,mx=-Infinity;
   const N=live?HIST:120,pad=1/(N-1),fa=z0-pad,fb=z1+pad;   // visible window, +1 sample each side
   for(let i=0;i<4;i++){const a=data[i];
    if(bars){for(let k=a.length-1;k>=0;k--){if(live||a[k]>=0){if(a[k]<mn)mn=a[k];if(a[k]>mx)mx=a[k];break;}}}
    else{for(let j=0;j<a.length;j++){const v=a[j];if(!live&&v<0)continue;
     const f=(live?j+N-a.length:j)/(N-1);if(f<fa||f>fb)continue;
     if(v<mn)mn=v;if(v>mx)mx=v;}}
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
   if(!w||!h)return;
   if(c.width!==Math.round(w*dpr)||c.height!==Math.round(h*dpr)){c.width=Math.round(w*dpr);c.height=Math.round(h*dpr);}
   ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,w,h);
   const cs=getComputedStyle(document.body);
   const cBorder=cs.getPropertyValue("--border").trim(),cMuted=cs.getPropertyValue("--muted").trim();
   const cols=[cs.getPropertyValue("--c0").trim(),cs.getPropertyValue("--c1").trim(),
               cs.getPropertyValue("--c2").trim(),cs.getPropertyValue("--c3").trim()];
   const u=UNITS[cur.metric];
   const pl=PL,pr=PR,pt=8,pb=16,pw=w-pl-pr,ph=h-pt-pb;
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

   // x position of sample j, mapped through the zoom window
   function xAt(j,len){const f=live?(j+HIST-len)/(HIST-1):j/(NP-1);return pl+pw*(f-z0)/(z1-z0);}

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
    const zm=zoomed();
    if(zm){ctx.save();ctx.beginPath();ctx.rect(pl,0,pw+pr,pt+ph+4);ctx.clip();}   // hide what's outside the zoom window
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
    if(zm)ctx.restore();
    ctx.fillStyle=cMuted;ctx.textAlign="left";
    let lbl,rlbl="now";
    if(!zm)lbl=live?("-"+(histT.length>1?((histT[histT.length-1]-histT[0])/1000).toFixed(1):"0")+"s"):HLBL[view];
    else{lbl=agoAt(z0,live);if(z1<0.999)rlbl=agoAt(z1,live);}
    ctx.fillText(lbl,pl+1,h-4);
    ctx.textAlign="right";ctx.fillText(rlbl,pl+pw,h-4);
   }
   ctx.textAlign="left";ctx.fillStyle=cMuted;ctx.fillText(u.s,4,h-4);
  }

  /* ---------- line chart zoom: wheel / pinch = zoom, drag = pan, double-tap or badge = reset ---------- */
  function zoomed(){return z1-z0<0.999;}
  function zMin(){return 6/((view===0?HIST:120)-1);}          // never show fewer than ~6 samples
  function zoomUi(){const on=cur.gtype===0&&zoomed();zBtn.classList.toggle("show",on);
   if(on)zBtn.textContent="\u21BA "+(1/(z1-z0)).toFixed(1)+"\u00D7";}
  function setWin(a,s){s=Math.min(1,Math.max(zMin(),s));a=Math.max(0,Math.min(1-s,a));
   if(a===z0&&a+s===z1)return;z0=a;z1=a+s;zoomUi();drawChart();}
  function resetZoom(redraw){z0=0;z1=1;zoomUi();if(redraw)drawChart();}
  function plotFrac(x){const r=cv.getBoundingClientRect();return Math.max(0,Math.min(1,(x-r.left-PL)/Math.max(1,r.width-PL-PR)));}
  function fmtAgo(s){const U=[[172800,86400,"d"],[7200,3600,"h"],[120,60,"m"],[0,1,"s"]];
   for(const q of U)if(s>=q[0]){const v=s/q[1];return "-"+(v>=10?Math.round(v):Math.round(v*10)/10)+q[2];}
   return "-0s";}
  function agoAt(f,live){
   if(!live)return fmtAgo(HDUR[view]*(1-f));
   const n=histT.length;if(n<2)return "-0.0s";
   const j=Math.max(0,Math.min(n-1,f*(HIST-1)-(HIST-n))),i=Math.floor(j),k=Math.min(n-1,i+1);
   const t=histT[i]+(histT[k]-histT[i])*(j-i);
   return "-"+((histT[n-1]-t)/1000).toFixed(1)+"s";}

  cv.addEventListener("wheel",function(e){
   if(cur.gtype!==0)return;
   e.preventDefault();
   const s=z1-z0,px=e.deltaMode===1?33:(e.deltaMode===2?cv.clientHeight:1),dx=e.deltaX*px,dy=e.deltaY*px;
   if(Math.abs(dx)>Math.abs(dy)){setWin(z0+dx/Math.max(1,cv.clientWidth-PL-PR)*s,s);return;}   // sideways scroll = pan
   const f=plotFrac(e.clientX),ns=Math.min(1,Math.max(zMin(),s*Math.exp(dy*(e.ctrlKey?0.01:0.0015))));
   setWin(z0+f*s-f*ns,ns);                                                                       // zoom around cursor
  },{passive:false});

  function startGest(){
   const v=Array.from(ptrs.values());
   if(v.length===1)gest={n:1,x:v[0].x,a:z0,s:z1-z0};
   else if(v.length>=2){multi=true;
    gest={n:2,d:Math.max(20,Math.hypot(v[0].x-v[1].x,v[0].y-v[1].y)),f:z0+plotFrac((v[0].x+v[1].x)/2)*(z1-z0),s:z1-z0};}
   else gest=null;
   plotEl.classList.toggle("drag",!!gest);
  }
  cv.addEventListener("pointerdown",function(e){
   if(cur.gtype!==0||(e.pointerType==="mouse"&&e.button!==0))return;
   try{cv.setPointerCapture(e.pointerId);}catch(_){}
   ptrs.set(e.pointerId,{x:e.clientX,y:e.clientY,x0:e.clientX,y0:e.clientY,t0:Date.now()});
   startGest();
  });
  cv.addEventListener("pointermove",function(e){
   const p=ptrs.get(e.pointerId);if(!p||!gest)return;
   p.x=e.clientX;p.y=e.clientY;
   const v=Array.from(ptrs.values());
   if(gest.n===1){setWin(gest.a-(p.x-gest.x)/Math.max(1,cv.clientWidth-PL-PR)*gest.s,gest.s);}
   else if(v.length>=2){
    const d=Math.max(20,Math.hypot(v[0].x-v[1].x,v[0].y-v[1].y)),f=plotFrac((v[0].x+v[1].x)/2),
          s=Math.min(1,Math.max(zMin(),gest.s*gest.d/d));
    setWin(gest.f-f*s,s);}
  });
  function endPtr(e){
   const p=ptrs.get(e.pointerId);if(!p)return;
   ptrs.delete(e.pointerId);
   if(ptrs.size===0){
    if(e.type==="pointerup"&&!multi&&Date.now()-p.t0<300&&Math.hypot(e.clientX-p.x0,e.clientY-p.y0)<10){
     const now=Date.now();
     if(now-lastTap<350&&Math.abs(e.clientX-lastTapX)<30){lastTap=0;resetZoom(true);}   // double-tap / double-click
     else{lastTap=now;lastTapX=e.clientX;}}
    multi=false;}
   startGest();
  }
  cv.addEventListener("pointerup",endPtr);
  cv.addEventListener("pointercancel",endPtr);
  cv.addEventListener("touchmove",function(e){if(cur.gtype===0&&e.touches.length>1)e.preventDefault();},{passive:false});
  cv.addEventListener("gesturestart",function(e){if(cur.gtype===0)e.preventDefault();});   // iOS: stop page zoom on pinch

  function clearHist(){if(view===0){hist=[[],[],[],[]];histT=[];}yLo=null;drawChart();}

  function downloadCsv(){window.location.href="/download";}
  function deleteCsv(){if(confirm(T[cur.lang].confirmDel)){fetch("/clear").then(function(){clearHist();});}}
  function setKeep(v){cur.keep=parseInt(v,10)||0;fetch("/set?keep="+cur.keep).catch(function(){});}

  async function poll(){
   if(busy)return false;busy=true;let ok=false;
   try{const d=await getJson("/data");
    cur.theme=d.theme;cur.lang=d.lang;cur.metric=d.metric;cur.auto=d.auto;cur.keep=d.keep;
    if(Date.now()-gLock>2000)cur.gtype=d.graphtype;
    applyTheme();applyLang();applyType();render(d);ok=true;}catch(e){}
   busy=false;return ok;
  }
  async function pollLoop(){const ok=await poll();setTimeout(pollLoop,ok&&view===0?0:POLL_MS);}
  function openPanel(){selTheme.value=cur.theme;selLang.value=cur.lang;selMetric.value=cur.metric;selGType.value=cur.gtype;
   chkAuto.checked=cur.auto;selMat.value=mat;selKeep.value=cur.keep;panel.classList.add("open");}
  function closePanel(){panel.classList.remove("open");}
  async function saveAndClose(){const q="theme="+selTheme.value+"&lang="+selLang.value+"&metric="+selMetric.value+"&graphtype="+selGType.value+"&auto="+(chkAuto.checked?1:0);
   await fetch("/set?"+q);panel.classList.remove("open");poll();}

  window.addEventListener("resize",drawChart);
  if(window.ResizeObserver){new ResizeObserver(function(){drawChart();}).observe(document.getElementById("plot"));}

  fetch("/time?t="+Math.floor(Date.now()/1000)).catch(function(){});
function tick(){const d=new Date(),p=function(n){return (n<10?"0":"")+n;};clock.textContent=p(d.getHours())+":"+p(d.getMinutes())+":"+p(d.getSeconds());}
applyMat(mat);applyType();pollLoop();tick();setInterval(tick,1000);
</script>
</body></html>
)HTML";