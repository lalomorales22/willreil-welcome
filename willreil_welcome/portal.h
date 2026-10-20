// portal.h — the welcome page itself. plain HTML/CSS/JS in one raw string.
// this lives in its own file on purpose: the Arduino IDE scans .ino files for
// function prototypes and chokes on the JavaScript "function foo(){" inside the
// page. headers are left alone. edit the copy/colors/sections however you like —
// no external assets allowed (there is no internet on the board's network).
//
// theme: the board itself. blue soldermask, white silkscreen, gold pads.
#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"~~~(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="format-detection" content="telephone=no">
<link rel="icon" href="data:,">
<title>your ESP32-C6</title>
<style>
:root{
  --pcb:#1c3a86;--pcb-deep:#102457;--pcb-ink:#0a1838;--pcb-edge:#2d56b4;
  --silk:#f1f4fa;--silk-70:rgba(241,244,250,.72);--silk-45:rgba(241,244,250,.45);--silk-20:rgba(241,244,250,.2);
  --gold:#e8c268;--gold-dim:#b8964a;--ok:#5cf0a2;--bad:#ff7a7a;
  --mono:ui-monospace,SFMono-Regular,Menlo,Consolas,"Liberation Mono",monospace
}
*{box-sizing:border-box}
html{-webkit-text-size-adjust:100%;background:var(--pcb)}
body{margin:0;color:var(--silk);font:16px/1.5 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif;padding:env(safe-area-inset-top) 0 calc(28px + env(safe-area-inset-bottom));
  background:var(--pcb) url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='200' height='200' viewBox='0 0 200 200'%3E%3Cg fill='none' stroke='%23ffffff' stroke-opacity='.07' stroke-width='2' stroke-linecap='round'%3E%3Cpath d='M12 34h52l22 22v44'/%3E%3Cpath d='M188 166h-52l-22-22v-44'/%3E%3Cpath d='M34 188v-34l22-22h34'/%3E%3Cpath d='M166 12v34l-22 22h-34'/%3E%3C/g%3E%3Cg fill='%23ffffff' fill-opacity='.09'%3E%3Ccircle cx='12' cy='34' r='4.5'/%3E%3Ccircle cx='86' cy='100' r='4.5'/%3E%3Ccircle cx='188' cy='166' r='4.5'/%3E%3Ccircle cx='114' cy='100' r='4.5'/%3E%3Ccircle cx='34' cy='188' r='4.5'/%3E%3Ccircle cx='90' cy='132' r='4.5'/%3E%3Ccircle cx='166' cy='12' r='4.5'/%3E%3Ccircle cx='110' cy='68' r='4.5'/%3E%3C/g%3E%3C/svg%3E") fixed}
.wrap{max-width:560px;margin:0 auto;padding:0 16px}
header{padding:30px 0 12px;border-bottom:1.5px dashed var(--silk-45)}
.eyebrow{font-family:var(--mono);font-size:12px;letter-spacing:.14em;text-transform:uppercase;display:flex;align-items:center;gap:10px;flex-wrap:wrap}
.silk{display:inline-block;border:1.5px solid var(--silk);border-radius:4px;padding:2px 8px;color:var(--silk);line-height:1.4}
.dot{width:9px;height:9px;border-radius:50%;background:var(--ok);box-shadow:0 0 0 0 rgba(92,240,162,.6);animation:pulse 1.8s ease-out infinite;flex:none}
.dot.off{background:var(--bad);animation:none}
@keyframes pulse{0%{box-shadow:0 0 0 0 rgba(92,240,162,.6)}100%{box-shadow:0 0 0 12px rgba(92,240,162,0)}}
h1{font-size:clamp(44px,14vw,72px);line-height:.92;letter-spacing:-.04em;margin:16px 0 12px;font-weight:800;color:#fff}
h1 span{color:var(--gold)}
.lede{color:var(--silk-70);margin:0 0 20px;font-size:17px}
.lede b{color:#fff;font-weight:600}
section{position:relative;border:1.5px solid var(--silk-70);background:rgba(8,20,58,.62);border-radius:14px;padding:20px 16px 18px;margin:16px 0}
section:before,section:after{content:"";position:absolute;width:9px;height:9px;border-radius:50%;border:2px solid var(--silk);background:var(--pcb-ink)}
section:before{top:6px;left:6px}section:after{bottom:6px;right:6px}
.lab{font-family:var(--mono);font-size:11px;letter-spacing:.14em;text-transform:uppercase;margin:0 0 12px;color:var(--gold)}
.lab b{font-weight:500;color:var(--silk);border:1.2px solid var(--silk-70);border-radius:3px;padding:1px 6px;margin-right:6px}
h2{font-size:22px;letter-spacing:-.02em;margin:0 0 8px;color:#fff}
p{margin:0 0 10px}
.muted{color:var(--silk-70)}
.small{font-size:14px}
dl{margin:0;display:grid;grid-template-columns:auto 1fr;row-gap:7px;column-gap:10px;font-size:15px;align-items:baseline}
dt{color:var(--silk-70);white-space:nowrap}
dd{margin:0;font-family:var(--mono);text-align:right;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;position:relative}
dd:before{content:"";position:absolute;left:0;right:0;bottom:6px;border-bottom:1.5px dotted var(--silk-20);z-index:0}
dd span{position:relative;background:#0e1f4f;padding-left:8px;z-index:1}
.row{display:flex;flex-wrap:wrap;gap:10px;margin:12px 0 8px;align-items:center}
button{appearance:none;-webkit-appearance:none;border:2px solid var(--gold-dim);background:var(--pcb-ink);color:var(--silk);font:inherit;font-weight:600;padding:10px 16px;border-radius:999px;cursor:pointer}
button:active{transform:translateY(1px)}
button.acc{background:var(--gold);border-color:var(--gold);color:#1a1400}
button.danger{border-color:#8a4a4a;color:#ffb0b0;background:transparent}
.sw{width:44px;height:44px;border-radius:50%;padding:0;border:3px solid var(--pcb-ink);box-shadow:0 0 0 2px var(--gold-dim)}
.sw.on{box-shadow:0 0 0 3px var(--gold),0 0 14px rgba(232,194,104,.5)}
.tabs{display:flex;gap:6px;flex-wrap:wrap;margin:0 0 14px}
.tabs button{padding:8px 13px;font-size:14px;background:transparent;border-color:var(--silk-45)}
.tabs button.on{background:var(--silk);color:var(--pcb-ink);border-color:var(--silk)}
.pane{display:none}.pane.on{display:block}
ol{padding-left:22px;margin:0}ol li{margin:0 0 10px}
code,pre{font-family:var(--mono);font-size:13.5px}
code{background:var(--pcb-ink);padding:2px 6px;border-radius:5px;white-space:nowrap;color:#fff}
pre{background:var(--pcb-ink);border:1.5px solid var(--silk-20);border-radius:10px;padding:12px;overflow:auto;margin:8px 0 10px;white-space:pre;-webkit-overflow-scrolling:touch;color:#fff}
pre.wrap{white-space:pre-wrap;word-break:break-all}
.cp{position:relative}
.cp button{position:absolute;top:14px;right:6px;padding:4px 10px;font-size:12px;background:var(--pcb-deep)}
.cp pre{padding-right:76px}
input[type=text]{width:100%;font:inherit;background:var(--pcb-ink);border:1.5px solid var(--silk-45);color:#fff;border-radius:10px;padding:12px 14px;margin:0 0 10px;outline:none}
input:focus{border-color:var(--gold)}
.msg{font-family:var(--mono);font-size:13px;color:var(--silk-70);min-height:20px;margin-top:6px}
.msg.ok{color:var(--ok)}.msg.bad{color:var(--bad)}
.warn{border-left:3px solid var(--gold);padding:8px 12px;background:rgba(232,194,104,.1);border-radius:0 10px 10px 0;font-size:14px;margin:12px 0}
.board{display:block;width:230px;max-width:100%;margin:6px auto 14px;filter:drop-shadow(0 10px 24px rgba(0,0,0,.35))}
.board text{font-family:var(--mono)}
.pingrid{display:grid;grid-template-columns:repeat(2,1fr);gap:8px;margin:0 0 12px}
.pin{background:var(--pcb-ink);border:1.5px solid var(--silk-20);border-radius:10px;padding:9px 12px;display:flex;flex-direction:column;gap:1px}
.pin b{font-family:var(--mono);font-weight:500;color:var(--silk-70);font-size:12.5px;letter-spacing:.04em}
.pin span{font-family:var(--mono);font-size:18px;color:var(--gold)}
footer{text-align:center;color:var(--silk-70);font-size:13px;padding:20px 0 0;line-height:1.9}
.mono{font-family:var(--mono);color:#fff}
.plate{display:inline-block;border:1.5px solid var(--silk-70);border-radius:6px;padding:8px 14px;font-family:var(--mono);font-size:11px;letter-spacing:.14em;text-transform:uppercase;color:var(--silk);margin:0 0 8px}
#overlay{display:none;position:fixed;inset:0;background:rgba(10,24,56,.97);padding:40px 24px;text-align:center}
#overlay.on{display:flex;flex-direction:column;justify-content:center;gap:12px}
#overlay .big{font-family:var(--mono);font-size:26px;color:var(--gold);word-break:break-all}
</style>
</head>
<body>
<div class="wrap">

<header>
  <div class="eyebrow"><span class="dot" id="dot"></span><span class="silk" id="brand">&nbsp;</span><span class="silk">unit <span id="unit">&mdash;</span></span></div>
  <h1>it's <span>alive.</span></h1>
  <p class="lede">you're connected straight to the chip. there's no internet on this network &mdash; it's just you and your new <b>ESP32&#8209;C6</b>, and this page is being served from it.</p>
</header>

<section>
  <p class="lab"><b>01</b>vitals &middot; live</p>
  <dl id="vitals"></dl>
</section>

<section>
  <p class="lab"><b>02</b>say hi</p>
  <h2>tap a color.</h2>
  <p class="muted small" id="ledhint">the LED on the board is wired to these buttons.</p>
  <div class="row" id="swatches"></div>
  <div class="row">
    <button onclick="led('fx=rainbow')">&#127752; rainbow</button>
    <button onclick="led('fx=off')">off</button>
  </div>
  <div class="msg" id="ledmsg"></div>
</section>

<section>
  <p class="lab"><b>03</b>flash your first program</p>
  <h2>make it yours.</h2>
  <p class="muted small">this welcome app lives in the chip's flash. the first time you upload your own code it's gone &mdash; that's the point. pick your tool:</p>
  <div class="tabs">
    <button class="on" onclick="tab(0,this)">Arduino IDE</button>
    <button onclick="tab(1,this)">PlatformIO</button>
    <button onclick="tab(2,this)">MicroPython</button>
    <button onclick="tab(3,this)">ESP-IDF</button>
  </div>

  <div class="pane on">
    <ol>
      <li>install <b>Arduino IDE 2</b> from <code>arduino.cc/en/software</code></li>
      <li><b>File &rarr; Preferences</b> &rarr; "Additional boards manager URLs" &rarr; paste this:
        <div class="cp"><pre class="wrap">https://espressif.github.io/arduino-esp32/package_esp32_index.json</pre><button onclick="cp(this)">copy</button></div></li>
      <li><b>Tools &rarr; Board &rarr; Boards Manager</b> &rarr; search <code>esp32</code> &rarr; install <b>esp32 by Espressif Systems</b> (version 3 or newer &mdash; older ones don't know the C6)</li>
      <li>plug the board in with a <b>data</b> USB&#8209;C cable (some cables are charge-only). <b>Tools &rarr; Board &rarr; esp32 &rarr; ESP32C6 Dev Module</b>, then <b>Tools &rarr; Port</b> &rarr; pick the one that just appeared</li>
      <li><b>Tools &rarr; USB CDC On Boot &rarr; Enabled</b>, so <code>Serial.print</code> shows up in the Serial Monitor over USB</li>
      <li><b>File &rarr; Examples &rarr; 01.Basics &rarr; Blink</b> &rarr; hit <b>Upload</b>. the LED blinks. you're a firmware developer now.</li>
    </ol>
    <p class="muted small">upload stuck on "Connecting..."? hold <b>BOOT</b>, tap <b>RESET</b>, let go of BOOT, upload again.</p>
  </div>

  <div class="pane">
    <p class="small">the stock <code>espressif32</code> platform in PlatformIO is stuck on the old 2.x core, which doesn't know the C6. use the <b>pioarduino</b> fork &mdash; it tracks the current core:</p>
    <div class="cp"><pre>[env:c6]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
board = esp32-c6-devkitc-1
framework = arduino
monitor_speed = 115200
build_flags = -DARDUINO_USB_CDC_ON_BOOT=1</pre><button onclick="cp(this)">copy</button></div>
    <p class="muted small">then <code>pio run -t upload</code> and <code>pio device monitor</code>.</p>
  </div>

  <div class="pane">
    <ol>
      <li>grab the C6 build from <code>micropython.org/download/ESP32_GENERIC_C6/</code></li>
      <li>flash it (the C6 boots from offset 0x0):
        <div class="cp"><pre>pip install esptool
python -m esptool --chip esp32c6 --port PORT erase_flash
python -m esptool --chip esp32c6 --port PORT write_flash 0x0 ESP32_GENERIC_C6-latest.bin</pre><button onclick="cp(this)">copy</button></div></li>
      <li>open <b>Thonny</b> (or <code>mpremote</code>), pick the port, and you've got a Python REPL on the chip:
        <div class="cp"><pre>from machine import Pin
import time
led = Pin(8, Pin.OUT)
while True:
    led.toggle()
    time.sleep(0.5)</pre><button onclick="cp(this)">copy</button></div></li>
    </ol>
  </div>

  <div class="pane">
    <p class="small">for the full Espressif toolchain (the stuff Arduino is built on):</p>
    <div class="cp"><pre>idf.py set-target esp32c6
idf.py menuconfig
idf.py build flash monitor</pre><button onclick="cp(this)">copy</button></div>
    <p class="muted small">install ESP-IDF 5.1 or newer first &mdash; the VS Code extension is the easy route.</p>
  </div>
</section>

<section>
  <p class="lab"><b>04</b>pins</p>
  <h2>where things are.</h2>
  <p class="muted small">this is your board. the LED in the drawing shows whatever color the real one is showing right now.</p>

  <svg class="board" viewBox="0 0 260 480" xmlns="http://www.w3.org/2000/svg" aria-label="board pinout">
    <rect x="18" y="12" width="224" height="456" rx="16" fill="#2148a2" stroke="#f1f4fa" stroke-width="2"/>
    <g fill="#0f2050" stroke="#f1f4fa" stroke-width="2">
      <circle cx="36" cy="30" r="9"/><circle cx="224" cy="30" r="9"/><circle cx="36" cy="450" r="9"/><circle cx="224" cy="450" r="9"/>
    </g>
    <g fill="#0a1638"><circle cx="36" cy="30" r="4"/><circle cx="224" cy="30" r="4"/><circle cx="36" cy="450" r="4"/><circle cx="224" cy="450" r="4"/></g>
    <g fill="none" stroke="#f1f4fa" stroke-opacity=".28" stroke-width="1.5" stroke-linecap="round">
      <path d="M70 286h34l14 14v36"/><path d="M190 298h-30l-12 12v26"/><path d="M118 390h24"/>
    </g>
    <g fill="#f1f4fa" fill-opacity=".35"><circle cx="118" cy="336" r="2.5"/><circle cx="148" cy="336" r="2.5"/><circle cx="118" cy="390" r="2.5"/><circle cx="142" cy="390" r="2.5"/></g>

    <rect x="72" y="24" width="116" height="106" rx="4" fill="#15161a" stroke="#6b6f78" stroke-width="1.5"/>
    <rect x="72" y="24" width="116" height="24" rx="4" fill="#2b2c33"/>
    <text x="130" y="72" font-size="7.5" fill="#9aa0ad" text-anchor="middle" letter-spacing="1">ESPRESSIF</text>
    <text x="130" y="88" font-size="9.5" fill="#e6e8ee" text-anchor="middle" font-weight="700">ESP32-C6-MINI-1</text>
    <rect x="96" y="100" width="68" height="18" rx="2" fill="none" stroke="#3a3c44" stroke-width="1" stroke-dasharray="2 2"/>

    <g>
      <rect x="80" y="144" width="22" height="22" rx="3" fill="#2b2d33" stroke="#7d8089" stroke-width="1.2"/><circle cx="91" cy="155" r="6" fill="#4a4d55"/>
      <rect x="158" y="144" width="22" height="22" rx="3" fill="#2b2d33" stroke="#7d8089" stroke-width="1.2"/><circle cx="169" cy="155" r="6" fill="#4a4d55"/>
      <rect x="106" y="146" width="48" height="16" rx="3" fill="none" stroke="#f1f4fa" stroke-width="1.2"/>
      <text x="130" y="157.5" font-size="7" fill="#f1f4fa" text-anchor="middle" font-weight="700">RESET | BOOT</text>
      <rect x="103" y="174" width="54" height="16" rx="3" fill="none" stroke="#f1f4fa" stroke-width="1.2"/>
      <text x="130" y="185.5" font-size="9" fill="#f1f4fa" text-anchor="middle" font-weight="700">ESP32-C6</text>
    </g>

    <g>
      <rect x="150" y="206" width="14" height="14" rx="2" fill="#0b1838" stroke="#8a8f99" stroke-width="1.2"/>
      <circle id="svg-led" cx="157" cy="213" r="5" fill="#ffaa28"/>
      <text id="svg-ledlbl" x="172" y="216.5" font-size="8.5" fill="#f1f4fa" font-weight="700">IO8</text>
    </g>

    <g fill="#0f2050" stroke="#e8c268" stroke-width="2.5">
      <circle cx="40" cy="140" r="7"/><circle cx="40" cy="168" r="7"/><circle cx="40" cy="196" r="7"/><circle cx="40" cy="224" r="7"/><circle cx="40" cy="252" r="7"/><circle cx="40" cy="280" r="7"/><circle cx="40" cy="308" r="7"/><circle cx="40" cy="336" r="7"/><circle cx="40" cy="364" r="7"/>
      <circle cx="220" cy="140" r="7"/><circle cx="220" cy="168" r="7"/><circle cx="220" cy="196" r="7"/><circle cx="220" cy="224" r="7"/><circle cx="220" cy="252" r="7"/><circle cx="220" cy="280" r="7"/><circle cx="220" cy="308" r="7"/><circle cx="220" cy="336" r="7"/><circle cx="220" cy="364" r="7"/><circle cx="220" cy="392" r="7"/><circle cx="220" cy="420" r="7"/>
    </g>
    <g fill="#061030">
      <circle cx="40" cy="140" r="3"/><circle cx="40" cy="168" r="3"/><circle cx="40" cy="196" r="3"/><circle cx="40" cy="224" r="3"/><circle cx="40" cy="252" r="3"/><circle cx="40" cy="280" r="3"/><circle cx="40" cy="308" r="3"/><circle cx="40" cy="336" r="3"/><circle cx="40" cy="364" r="3"/>
      <circle cx="220" cy="140" r="3"/><circle cx="220" cy="168" r="3"/><circle cx="220" cy="196" r="3"/><circle cx="220" cy="224" r="3"/><circle cx="220" cy="252" r="3"/><circle cx="220" cy="280" r="3"/><circle cx="220" cy="308" r="3"/><circle cx="220" cy="336" r="3"/><circle cx="220" cy="364" r="3"/><circle cx="220" cy="392" r="3"/><circle cx="220" cy="420" r="3"/>
    </g>
    <g font-size="9" fill="#f1f4fa" font-weight="700">
      <text x="54" y="143.5">7</text><text x="54" y="171.5">6</text><text x="54" y="199.5">5</text><text x="54" y="227.5">4</text><text x="54" y="255.5">3</text><text x="54" y="283.5">2</text><text x="54" y="311.5">1</text>
      <text id="svg-sda" x="54" y="339.5">0/SDA</text><text id="svg-scl" x="54" y="367.5">14/SCL</text>
      <text x="206" y="143.5" text-anchor="end">23</text><text x="206" y="171.5" text-anchor="end">22</text><text x="206" y="199.5" text-anchor="end">21</text><text x="206" y="227.5" text-anchor="end">20</text><text x="206" y="255.5" text-anchor="end">19</text><text x="206" y="283.5" text-anchor="end">18</text><text x="206" y="311.5" text-anchor="end">17</text><text x="206" y="339.5" text-anchor="end">16</text>
      <text x="206" y="367.5" text-anchor="end" fill="#e8c268">3V3</text><text x="206" y="395.5" text-anchor="end" fill="#e8c268">GND</text><text x="206" y="423.5" text-anchor="end" fill="#e8c268">5V</text>
    </g>

    <rect x="28" y="388" width="30" height="40" rx="3" fill="#e9ecf2" stroke="#a9afbb" stroke-width="1.2"/>
    <g fill="#9aa0ad"><rect x="33" y="394" width="4" height="22"/><rect x="40" y="394" width="4" height="22"/><rect x="47" y="394" width="4" height="22"/><rect x="54" y="394" width="2" height="22"/></g>
    <text x="43" y="438" font-size="7.5" fill="#f1f4fa" text-anchor="middle" font-weight="700">I2C</text>

    <rect x="104" y="416" width="52" height="34" rx="9" fill="#c8ccd4" stroke="#8a8f99" stroke-width="1.5"/>
    <rect x="113" y="427" width="34" height="12" rx="6" fill="#6d727c"/>
    <text x="130" y="410" font-size="7.5" fill="#f1f4fa" text-anchor="middle" font-weight="700">USB-C</text>

    <text id="svg-brand" x="130" y="250" font-size="11" fill="#f1f4fa" text-anchor="middle" font-weight="800" letter-spacing="1.5">REIL INDUSTRIES</text>
    <text id="svg-unit" x="130" y="265" font-size="8.5" fill="#f1f4fa" fill-opacity=".8" text-anchor="middle" letter-spacing="1.5">UNIT</text>
  </svg>

  <div class="pingrid" id="pins"></div>
  <p class="muted small">the white 4-pin jack next to <span class="mono">0/SDA &middot; 14/SCL</span> is the I2C port (Qwiic / STEMMA QT style): plug a sensor straight in, no soldering. in Arduino that's <code id="wire">Wire.begin(0, 14);</code></p>
  <div id="c6notes">
    <div class="warn"><b>strapping pins:</b> GPIO 4, 5, 8, 9 and 15 get read by the chip at reset to decide how to boot. leave them floating or pulled correctly during power-up or you'll get weird boot behavior. GPIO 9 is the BOOT button.</div>
    <p class="muted small">analog in: GPIO 0&ndash;6 (ADC1). native USB: GPIO 12 (D&minus;) and 13 (D+) &mdash; that's your USB&#8209;C port, don't repurpose them. UART0 lives on GPIO 16 (TX) / 17 (RX). RISC&#8209;V core at 160 MHz plus a 20 MHz low-power core, 512 KB SRAM, Wi&#8209;Fi 6, Bluetooth 5 LE, and 802.15.4 for Zigbee / Thread.</p>
  </div>
</section>

<section>
  <p class="lab"><b>05</b>this wifi</p>
  <h2>make the password yours.</h2>
  <p class="muted small">the password on the card is fine, but you can change it. the board restarts and you reconnect with the new one. forgot it? hold <b>BOOT</b> for 5 seconds and it goes back to the card password.</p>
  <input type="text" id="pw" placeholder="new password (8 to 63 characters)" autocomplete="off" autocapitalize="off" spellcheck="false">
  <div class="row">
    <button class="acc" onclick="setpw()">save &amp; restart</button>
    <button class="danger" id="rst" onclick="resetpw(this)">back to card password</button>
  </div>
  <div class="msg" id="pwmsg"></div>
</section>

<section>
  <p class="lab"><b>06</b>when you're done here</p>
  <h2>stop the sign-in popup.</h2>
  <p class="muted small">your phone thinks this is hotel wifi and will keep asking you to "sign in". tap below and it'll stop. you stay connected, and you can always come back to this page at <span class="mono" id="urlhint">http://4.3.2.1</span> (http, not https &mdash; there's no internet to get a certificate from).</p>
  <div class="row"><button class="acc" onclick="release()">got it, stop asking</button></div>
  <div class="msg" id="relmsg"></div>
</section>

<footer>
  <div class="plate" id="plate">property of whoever's holding it</div>
  <div id="contact"></div>
  <div>the links above are text on purpose &mdash; you're on the board's network, so nothing on the internet loads. switch back to your normal wifi and type them in.</div>
  <div class="muted">&#9679; reflash <span class="mono">willreil_welcome.ino</span> any time to get this page back</div>
</footer>

</div>

<div id="overlay">
  <div class="lab">rebooting</div>
  <h2 id="ovtitle">saved.</h2>
  <p class="muted">join the wifi again &mdash; same network name, new password:</p>
  <div class="big" id="ovssid"></div>
  <p class="muted small">then open <span class="mono" id="ovurl"></span></p>
</div>

<script>
var S = null, TXT = 0;
function $(i){ return document.getElementById(i); }
function h(s){ return String(s).replace(/[&<>"]/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]; }); }
function up(s){ var d=Math.floor(s/86400), hh=Math.floor(s%86400/3600), m=Math.floor(s%3600/60), x=s%60;
  return (d?d+'d ':'')+(hh?hh+'h ':'')+(m?m+'m ':'')+x+'s'; }
function post(u, body){
  return fetch(u, {method:'POST', cache:'no-store', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body: body||''})
    .then(function(r){ return r.json(); });
}
function row(k, v){ return '<dt>'+k+'</dt><dd><span>'+v+'</span></dd>'; }

function render(s){
  S = s;
  $('dot').className = 'dot';
  $('brand').textContent = s.brand;
  $('unit').textContent = s.unit;
  $('urlhint').textContent = 'http://' + s.ip;
  $('ovurl').textContent = 'http://' + s.ip;
  $('ovssid').textContent = s.ssid;

  $('vitals').innerHTML =
    row('chip', h(s.chip) + ' rev ' + s.rev) +
    row('cpu', s.cores + ' core &middot; ' + s.mhz + ' MHz') +
    row('flash', s.flashMB + ' MB (' + s.sketchKB + ' KB used)') +
    row('ram', s.freeHeapKB + ' / ' + s.heapKB + ' KB free') +
    row('temp', s.tempC.toFixed(1) + ' &deg;C') +
    row('uptime', up(s.uptime)) +
    row('wifi', h(s.ssid) + ' &middot; ch ' + s.channel) +
    row('connected', s.clients + (s.clients == 1 ? ' device (you)' : ' devices')) +
    row('your ip', s.you) +
    row('board mac', s.mac) +
    row('arduino core', 'v' + s.core) +
    row('esp-idf', h(s.idf));

  var p = s.pins;
  $('pins').innerHTML =
    pin('UART TX', p.tx) + pin('UART RX', p.rx) +
    pin('I2C SDA', p.sda) + pin('I2C SCL', p.scl) +
    pin('SPI MOSI', p.mosi) + pin('SPI MISO', p.miso) +
    pin('SPI SCK', p.sck) + pin('SPI SS', p.ss) +
    pin('BOOT button', p.boot) +
    pin(s.led.kind == 1 ? 'RGB LED (WS2812)' : 'LED', s.led.kind == 3 ? 'none' : s.led.pin);
  $('wire').textContent = 'Wire.begin(' + p.sda + ', ' + p.scl + ');';
  $('c6notes').style.display = (s.chip.indexOf('C6') >= 0) ? '' : 'none';

  // the board drawing
  $('svg-sda').textContent = p.sda + '/SDA';
  $('svg-scl').textContent = p.scl + '/SCL';
  $('svg-ledlbl').textContent = s.led.kind == 3 ? '' : 'IO' + s.led.pin;
  var bn = s.brand.toUpperCase(); $('svg-brand').textContent = bn;
  $('svg-brand').setAttribute('font-size', bn.length > 20 ? 6.5 : bn.length > 12 ? 7.5 : 11);
  $('svg-brand').setAttribute('letter-spacing', bn.length > 12 ? 1 : 1.5);
  $('svg-unit').textContent = 'UNIT ' + s.unit;
  var ledEl = $('svg-led');
  if (s.led.fx == 'rainbow') { ledEl.setAttribute('fill', '#ff3cac'); ledEl.style.animation = 'spin 3.6s linear infinite'; }
  else {
    ledEl.style.animation = '';
    var off = !(s.led.r | s.led.g | s.led.b);
    ledEl.setAttribute('fill', off ? '#1a2350' : 'rgb(' + s.led.r + ',' + s.led.g + ',' + s.led.b + ')');
  }

  if (!TXT) {
    TXT = 1;
    var c = [];
    if (s.x)     c.push('<span class="mono">' + h(s.x) + '</span>');
    if (s.site)  c.push('<span class="mono">' + h(s.site) + '</span>');
    if (s.email) c.push('<span class="mono">' + h(s.email) + '</span>');
    $('contact').innerHTML = 'questions, ideas, show-offs &rarr; ' + c.join(' &middot; ');
    $('plate').textContent = h(s.brand) + ' · unit ' + s.unit + ' · property of whoever’s holding it';
    if (s.led.kind == 3) $('ledhint').textContent = 'this build has no LED configured, so the buttons just ping the board.';
    else $('ledhint').textContent = 'the LED on GPIO ' + s.led.pin + ' is wired to these buttons. that round trip is phone → wifi → chip → light.';
    if (s.released) $('relmsg').textContent = 'already done for this device.';
    if (s.customPw) $('pwmsg').textContent = 'this board is using a customer-set password.';
  }
  markSwatch(s.led.fx == 'solid' ? [s.led.r, s.led.g, s.led.b].join(',') : null);
}
function pin(name, v){ return '<div class="pin"><b>' + name + '</b><span>' + (v === 'none' ? v : 'GPIO ' + v) + '</span></div>'; }

function tick(){
  fetch('/api/status', {cache:'no-store'}).then(function(r){ return r.json(); }).then(render)
    .catch(function(){ $('dot').className = 'dot off'; })
    .then(function(){ setTimeout(tick, 3000); });
}

var SW = [[255,170,40],[255,40,40],[40,255,90],[40,120,255],[255,255,255],[255,0,200],[0,230,230]];
(function(){
  var html = '';
  SW.forEach(function(c){
    html += '<button class="sw" data-c="' + c.join(',') + '" style="background:rgb(' + c.join(',') + ')" onclick="led(\'fx=solid&r=' + c[0] + '&g=' + c[1] + '&b=' + c[2] + '\',this)"></button>';
  });
  $('swatches').innerHTML = html;
  var st = document.createElement('style');
  st.textContent = '@keyframes spin{from{filter:hue-rotate(0deg)}to{filter:hue-rotate(360deg)}}';
  document.head.appendChild(st);
})();
function markSwatch(key){
  var b = document.querySelectorAll('.sw');
  for (var i = 0; i < b.length; i++) b[i].className = 'sw' + (b[i].getAttribute('data-c') === key ? ' on' : '');
}
function led(q, btn){
  $('ledmsg').className = 'msg'; $('ledmsg').textContent = '...';
  post('/api/led', q).then(function(){
    $('ledmsg').className = 'msg ok'; $('ledmsg').textContent = 'board says: ok. look at it.';
    markSwatch(btn ? btn.getAttribute('data-c') : null);
  }).catch(function(){ $('ledmsg').className = 'msg bad'; $('ledmsg').textContent = 'no answer from the board'; });
}

function tab(n, btn){
  var t = document.querySelectorAll('.tabs button'), p = document.querySelectorAll('.pane');
  for (var i = 0; i < t.length; i++) { t[i].className = (i == n) ? 'on' : ''; p[i].className = 'pane' + ((i == n) ? ' on' : ''); }
}

function cp(b){
  var pre = b.parentNode.querySelector('pre'), t = pre.textContent;
  function done(w){ b.textContent = w; setTimeout(function(){ b.textContent = 'copy'; }, 1400); }
  function select(){
    var r = document.createRange(); r.selectNodeContents(pre);
    var s = window.getSelection(); s.removeAllRanges(); s.addRange(r);
    try { if (document.execCommand('copy')) { done('copied'); return; } } catch(e){}
    done('selected');
  }
  if (navigator.clipboard && navigator.clipboard.writeText) navigator.clipboard.writeText(t).then(function(){ done('copied'); }, select);
  else select();
}

function setpw(){
  var pw = $('pw').value, m = $('pwmsg');
  m.className = 'msg';
  if (pw.length < 8 || pw.length > 63) { m.className = 'msg bad'; m.textContent = '8 to 63 characters'; return; }
  m.textContent = 'saving...';
  post('/api/setpass', 'pw=' + encodeURIComponent(pw)).then(function(r){
    if (!r.ok) { m.className = 'msg bad'; m.textContent = r.err || 'nope'; return; }
    $('ovtitle').textContent = 'saved.';
    $('overlay').className = 'on';
  }).catch(function(){ m.className = 'msg bad'; m.textContent = 'no answer from the board'; });
}
function resetpw(b){
  if (b.getAttribute('data-arm') !== '1') { b.setAttribute('data-arm', '1'); b.textContent = 'tap again to confirm'; setTimeout(function(){ b.setAttribute('data-arm', '0'); b.textContent = 'back to card password'; }, 4000); return; }
  post('/api/resetpass').then(function(){
    $('ovtitle').textContent = 'reset.';
    document.querySelector('#overlay p').textContent = 'join the wifi again with the password on your card:';
    $('overlay').className = 'on';
  }).catch(function(){ $('pwmsg').className = 'msg bad'; $('pwmsg').textContent = 'no answer from the board'; });
}
function release(){
  post('/api/release').then(function(){
    $('relmsg').className = 'msg ok'; $('relmsg').textContent = 'done. this device won\'t be nagged anymore. see you at http://' + (S ? S.ip : '4.3.2.1');
  }).catch(function(){ $('relmsg').className = 'msg bad'; $('relmsg').textContent = 'no answer from the board'; });
}
tick();
</script>
</body>
</html>)~~~";
