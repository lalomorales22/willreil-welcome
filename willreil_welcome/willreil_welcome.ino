/*
  willreil_welcome.ino  —  "it's alive" welcome portal for ESP32-C6 boards
  ==========================================================================

  Flash this on every board before it ships. When the buyer plugs it in:

    1. the board broadcasts its own wifi            ->  WillReil-7F3A
    2. their phone joins with the password on the card
    3. the phone pops the "sign in to network" sheet (hotel-wifi style)
    4. that sheet IS this app: live chip stats, poke-the-LED, and
       step-by-step first-flash instructions for Arduino / PlatformIO /
       MicroPython / ESP-IDF

  FILES
    willreil_welcome.ino  — the firmware (this file)
    portal.h              — the page the customer sees (HTML/CSS/JS in one string)

  BUILD
    Arduino IDE 2.x  +  "esp32 by Espressif Systems" core 3.x (C6 needs 3.x)
    Tools -> Board          : ESP32C6 Dev Module
    Tools -> USB CDC On Boot: Enabled
    Zero libraries to install — only stuff that ships inside the core.

  SHIP (per board, ~20 seconds)
    Upload, open Serial Monitor @ 115200, press RESET (or type anything + Enter).
    The board prints:
          SSID : WillReil-7F3A
          PASS : 7K3M-9PXZ
    Write those two lines on the card in the box. That's the whole process.
    (ship.py in this folder does flash + label automatically, with a CSV log.)

  HOW PASSWORDS WORK
    PER_UNIT_PASSWORD 1  -> every board derives its own password from its
                            factory MAC + UNIT_SECRET (sha256). Same firmware
                            on every board, different password on every card.
    PER_UNIT_PASSWORD 0  -> every board uses FIXED_PASSWORD.
    Buyers can set their own password from the app (saved to NVS).
    Hold BOOT for 5 s while running -> forgets the custom password, back to card.

  NETWORK
    AP ip 4.3.2.1 / DNS hijack ("*" -> 4.3.2.1) / mDNS http://willreil.local
    Any http:// address a phone tries lands here. https:// can't (by design).
*/

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_arduino_version.h>
#include <mbedtls/sha256.h>
#include <soc/soc_caps.h>

// ================================ CONFIG ==================================
#define BRAND             "Will Reil Industries"
#define SSID_PREFIX       "WillReil"           // network shows up as WillReil-7F3A
#define HOSTNAME          "willreil"           // http://willreil.local also works
#define CONTACT_X         "x.com/realwillreil"
#define CONTACT_SITE      ""                   // e.g. "willreil.com"  ("" hides it)
#define CONTACT_EMAIL     ""                   // e.g. "hi@willreil.com" ("" hides it)
// (no double-quotes inside any of the strings above — they go straight into JSON)

#define PER_UNIT_PASSWORD 1                    // 1 = unique password per board
#define FIXED_PASSWORD    "willreil2026"       // only used when PER_UNIT_PASSWORD is 0
#define UNIT_SECRET       "wri-batch-2026"     // salt for per-unit passwords.
                                               // change it ONCE before the first batch,
                                               // then never — or old cards stop matching.

// LED. AUTO = trust the board variant (C6 DevKit: WS2812 RGB on GPIO8).
// If Will's own board puts the LED somewhere else, pick a mode + pin here.
#define LED_MODE_AUTO     0
#define LED_MODE_RGB      1                    // WS2812 / NeoPixel on LED_PIN
#define LED_MODE_PLAIN    2                    // ordinary LED on LED_PIN
#define LED_MODE_NONE     3
#define LED_MODE          LED_MODE_AUTO
#define LED_PIN           8
#define LED_ACTIVE_LOW    0                    // PLAIN only: 1 if LOW turns it on
#define LED_BRIGHTNESS    64                   // RGB only, 0-255. 64 is plenty.

#define BOOT_BTN          9                    // BOOT button on every C6 dev board

// What the silkscreen says, not what the Arduino core assumes. Will's boards break
// I2C out on 0/SDA + 14/SCL (and the Qwiic jack); the core's default is 6/7.
// -1 = fall back to the core's default for the selected board.
#define BOARD_SDA         0
#define BOARD_SCL         14

#define AP_CHANNEL        6
#define AP_MAX_CLIENTS    8
// ==========================================================================

static_assert(sizeof(FIXED_PASSWORD) - 1 >= 8, "FIXED_PASSWORD must be at least 8 characters (WPA2)");
static_assert(sizeof(FIXED_PASSWORD) - 1 <= 63, "FIXED_PASSWORD must be 63 characters or fewer");

// ---- resolve which LED we actually drive -----------------------------------
#if LED_MODE == LED_MODE_AUTO
  #if defined(RGB_BUILTIN)
    #define LED_KIND 1
    #define LED_GPIO RGB_BUILTIN
  #elif defined(LED_BUILTIN)
    #define LED_KIND 2
    #define LED_GPIO LED_BUILTIN
  #else
    #define LED_KIND 3
    #define LED_GPIO -1
  #endif
#elif LED_MODE == LED_MODE_RGB
  #define LED_KIND 1
  #define LED_GPIO LED_PIN
#elif LED_MODE == LED_MODE_PLAIN
  #define LED_KIND 2
  #define LED_GPIO LED_PIN
#else
  #define LED_KIND 3
  #define LED_GPIO -1
#endif

// ---- globals ---------------------------------------------------------------
static const IPAddress AP_MASK(255, 255, 255, 0);
static IPAddress portalIp(4, 3, 2, 1);           // replaced with the real AP ip at boot

static DNSServer   dns;
static WebServer   server(80);
static Preferences prefs;

static String   apSsid, apPass;
static bool     customPw   = false;
static uint32_t restartAt  = 0;                  // millis() deadline, 0 = none
static volatile bool blipRequest = false;        // set from wifi event task, handled in loop

// clients that clicked "stop the sign-in popup" — probes get real answers afterwards
static IPAddress released[8];
static uint8_t   releasedNext = 0;

// ============================== LED ========================================
enum Fx : uint8_t { FX_SOLID, FX_RAINBOW };
static Fx       fx = FX_SOLID;
static uint8_t  solidR = 0, solidG = 0, solidB = 0;
static uint16_t hue = 0;
static uint32_t fxLast = 0, blipUntil = 0;

static int ledPhysicalPin() {
#if LED_KIND == 3
  return -1;
#else
  int p = LED_GPIO;
  if (p >= SOC_GPIO_PIN_COUNT) p -= SOC_GPIO_PIN_COUNT;   // variant "magic" RGB pin encoding
  return p;
#endif
}

static void ledWrite(uint8_t r, uint8_t g, uint8_t b) {
#if LED_KIND == 1
  r = (uint16_t)r * LED_BRIGHTNESS / 255;
  g = (uint16_t)g * LED_BRIGHTNESS / 255;
  b = (uint16_t)b * LED_BRIGHTNESS / 255;
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 1, 0)
    rgbLedWrite(LED_GPIO, r, g, b);
  #else
    neopixelWrite(LED_GPIO, r, g, b);
  #endif
#elif LED_KIND == 2
  bool on = (r | g | b) != 0;
  digitalWrite(LED_GPIO, (on != (bool)LED_ACTIVE_LOW) ? HIGH : LOW);
#else
  (void)r; (void)g; (void)b;
#endif
}

static void ledInit() {
#if LED_KIND == 2
  pinMode(LED_GPIO, OUTPUT);
#endif
  ledWrite(0, 0, 0);
}

static void hsvToRgb(uint16_t h, uint8_t &r, uint8_t &g, uint8_t &b) {
  uint8_t sector = h / 60;
  uint8_t f = (uint8_t)((h % 60) * 255 / 60);
  uint8_t q = 255 - f;
  switch (sector) {
    case 0:  r = 255; g = f;   b = 0;   break;
    case 1:  r = q;   g = 255; b = 0;   break;
    case 2:  r = 0;   g = 255; b = f;   break;
    case 3:  r = 0;   g = q;   b = 255; break;
    case 4:  r = f;   g = 0;   b = 255; break;
    default: r = 255; g = 0;   b = q;   break;
  }
}

static void ledSetSolid(uint8_t r, uint8_t g, uint8_t b) {
  fx = FX_SOLID; solidR = r; solidG = g; solidB = b;
  ledWrite(r, g, b);
}

static void ledTick() {
  uint32_t now = millis();

  if (blipRequest) {                       // someone just joined: green blip
    blipRequest = false;
    if (fx == FX_SOLID) { ledWrite(0, 255, 0); blipUntil = now + 400; }
  }
  if (blipUntil) {
    if (now < blipUntil) return;
    blipUntil = 0;
    ledWrite(solidR, solidG, solidB);
  }
  if (fx == FX_RAINBOW && now - fxLast >= 20) {
    fxLast = now;
    hue = (hue + 2) % 360;                 // full rotation every 3.6 s
#if LED_KIND == 2
    ledWrite(hue < 180 ? 255 : 0, 0, 0);   // plain LED can't do rainbow: it blinks
#else
    uint8_t r, g, b; hsvToRgb(hue, r, g, b);
    ledWrite(r, g, b);
#endif
  }
}

// ============================ identity =====================================
static String unitId() {                   // last two bytes of the factory MAC -> "7F3A"
  uint64_t mac = ESP.getEfuseMac();
  const uint8_t *m = (const uint8_t *)&mac;
  char s[5];
  snprintf(s, sizeof s, "%02X%02X", m[4], m[5]);
  return String(s);
}

static String derivedPassword() {          // sha256(UNIT_SECRET + mac) -> "7K3M-9PXZ"
  uint64_t mac = ESP.getEfuseMac();
  uint8_t in[sizeof(UNIT_SECRET) - 1 + sizeof mac];
  memcpy(in, UNIT_SECRET, sizeof(UNIT_SECRET) - 1);
  memcpy(in + sizeof(UNIT_SECRET) - 1, &mac, sizeof mac);
  uint8_t h[32];
  (void)mbedtls_sha256(in, sizeof in, h, 0);
  static const char A[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";   // 32 symbols, no 0/O/1/I
  String p;
  for (int i = 0; i < 8; i++) {
    if (i == 4) p += '-';
    p += A[h[i] & 31];
  }
  return p;
}

static void loadCreds() {
  String prefix = SSID_PREFIX;
  if (prefix.length() > 27) prefix = prefix.substring(0, 27);   // SSID max 32: prefix + "-XXXX"
  apSsid = prefix + "-" + unitId();

  String custom = prefs.getString("appw", "");
  if (custom.length() >= 8 && custom.length() <= 63) {
    apPass = custom; customPw = true;
  } else {
#if PER_UNIT_PASSWORD
    apPass = derivedPassword();
#else
    apPass = FIXED_PASSWORD;
#endif
    customPw = false;
  }
}

static void printBanner() {
  Serial.println();
  Serial.println("============================================");
  Serial.printf ("  %s  -  welcome portal\n", BRAND);
  Serial.println("============================================");
  Serial.printf ("  SSID : %s\n", apSsid.c_str());
  Serial.printf ("  PASS : %s   %s\n", apPass.c_str(), customPw ? "(customer-set)" : "(card password)");
  Serial.printf ("  URL  : http://%s/   or   http://%s.local/\n", portalIp.toString().c_str(), HOSTNAME);
  Serial.printf ("  MAC  : %s   unit %s\n", WiFi.softAPmacAddress().c_str(), unitId().c_str());
  Serial.println("--------------------------------------------");
  Serial.println("  >> write SSID + PASS on the card in the box <<");
  Serial.println("  (type anything + Enter to print this again)");
  Serial.printf ("LABEL|%s|%s|%s\n\n", apSsid.c_str(), apPass.c_str(), WiFi.softAPmacAddress().c_str());
}

// ======================= captive-portal plumbing ===========================
static bool isReleased(const IPAddress &ip) {
  for (auto &r : released) if (r == ip) return true;
  return false;
}
static void releaseClient(const IPAddress &ip) {
  if (isReleased(ip)) return;
  released[releasedNext] = ip;
  releasedNext = (releasedNext + 1) % (sizeof released / sizeof released[0]);
}

static bool hostIsUs() {
  String h = server.hostHeader();
  h.toLowerCase();
  int colon = h.indexOf(':');
  if (colon > 0) h = h.substring(0, colon);
  return h == portalIp.toString() || h == String(HOSTNAME) + ".local" || h == HOSTNAME;
}

static void redirectToPortal() {
  server.sendHeader("Location", "http://" + portalIp.toString() + "/", true);
  server.sendHeader("Cache-Control", "no-store");
  server.send(302, "text/plain", "");
  server.client().stop();
}

// OS connectivity probes. Not released -> redirect (this is what pops the sheet).
// Released -> answer exactly what each OS wants to hear, so it stops nagging.
static void handleProbe() {
  if (!isReleased(server.client().remoteIP())) { redirectToPortal(); return; }
  String u = server.uri();
  server.sendHeader("Cache-Control", "no-store");
  if (u.endsWith("generate_204") || u.endsWith("gen_204")) {                // android / chrome
    server.send(204, "text/plain", "");
  } else if (u.indexOf("hotspot-detect") >= 0 || u.indexOf("success.html") >= 0) {   // apple
    server.send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
  } else if (u.endsWith("connecttest.txt")) {                               // windows
    server.send(200, "text/plain", "Microsoft Connect Test");
  } else if (u.endsWith("ncsi.txt")) {
    server.send(200, "text/plain", "Microsoft NCSI");
  } else if (u.endsWith("success.txt")) {                                   // firefox
    server.send(200, "text/plain", "success\n");
  } else {
    server.send(200, "text/plain", "ok");
  }
}

static void handleNotFound() {
  if (!hostIsUs()) { redirectToPortal(); return; }
  server.send(404, "text/plain", "nothing here. try /");
}

// ============================== API ========================================
#include "portal.h"                        // the page itself (INDEX_HTML)

static void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", INDEX_HTML);
}

static void handleStatus() {
  IPAddress me = server.client().remoteIP();
  int lp = ledPhysicalPin();
  float tempC = temperatureRead();
  if (tempC != tempC) tempC = 0;             // NaN check without isnan(): "nan" would break the JSON
  char buf[1600];
  snprintf(buf, sizeof buf,
    "{\"brand\":\"%s\",\"ssid\":\"%s\",\"unit\":\"%s\",\"host\":\"%s\",\"ip\":\"%s\","
    "\"x\":\"%s\",\"site\":\"%s\",\"email\":\"%s\","
    "\"chip\":\"%s\",\"rev\":%u,\"cores\":%u,\"mhz\":%u,\"flashMB\":%u,"
    "\"sketchKB\":%u,\"freeSketchKB\":%u,\"heapKB\":%u,\"freeHeapKB\":%u,"
    "\"tempC\":%.1f,\"uptime\":%u,\"clients\":%u,\"channel\":%d,\"mac\":\"%s\","
    "\"core\":\"%d.%d.%d\",\"idf\":\"%s\","
    "\"pins\":{\"tx\":%d,\"rx\":%d,\"sda\":%d,\"scl\":%d,\"mosi\":%d,\"miso\":%d,\"sck\":%d,\"ss\":%d,\"boot\":%d},"
    "\"led\":{\"kind\":%d,\"pin\":%d,\"fx\":\"%s\",\"r\":%u,\"g\":%u,\"b\":%u},"
    "\"customPw\":%s,\"released\":%s,\"you\":\"%s\"}",
    BRAND, apSsid.c_str(), unitId().c_str(), HOSTNAME, portalIp.toString().c_str(),
    CONTACT_X, CONTACT_SITE, CONTACT_EMAIL,
    ESP.getChipModel(), (unsigned)ESP.getChipRevision(), (unsigned)ESP.getChipCores(),
    (unsigned)ESP.getCpuFreqMHz(), (unsigned)(ESP.getFlashChipSize() / (1024UL * 1024UL)),
    (unsigned)(ESP.getSketchSize() / 1024), (unsigned)(ESP.getFreeSketchSpace() / 1024),
    (unsigned)(ESP.getHeapSize() / 1024), (unsigned)(ESP.getFreeHeap() / 1024),
    tempC, (unsigned)(millis() / 1000), (unsigned)WiFi.softAPgetStationNum(),
    (int)WiFi.channel(), WiFi.softAPmacAddress().c_str(),
    ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH, ESP.getSdkVersion(),
    (int)TX, (int)RX, (BOARD_SDA >= 0 ? BOARD_SDA : (int)SDA), (BOARD_SCL >= 0 ? BOARD_SCL : (int)SCL),
    (int)MOSI, (int)MISO, (int)SCK, (int)SS, BOOT_BTN,
    LED_KIND, lp, fx == FX_RAINBOW ? "rainbow" : "solid", solidR, solidG, solidB,
    customPw ? "true" : "false", isReleased(me) ? "true" : "false", me.toString().c_str());
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", buf);
}

static void handleLed() {
  String mode = server.arg("fx");
  if (mode == "rainbow") {
    fx = FX_RAINBOW;
  } else if (mode == "off") {
    ledSetSolid(0, 0, 0);
  } else {
    int r = constrain(server.arg("r").toInt(), 0, 255);
    int g = constrain(server.arg("g").toInt(), 0, 255);
    int b = constrain(server.arg("b").toInt(), 0, 255);
    ledSetSolid(r, g, b);
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleSetPass() {
  String pw = server.arg("pw");
  if (pw.length() < 8 || pw.length() > 63) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"8 to 63 characters\"}");
    return;
  }
  for (size_t i = 0; i < pw.length(); i++) {
    char c = pw[i];
    if (c < 0x20 || c > 0x7E) {
      server.send(400, "application/json", "{\"ok\":false,\"err\":\"plain letters, numbers and symbols only\"}");
      return;
    }
  }
  prefs.putString("appw", pw);
  Serial.println("[wifi] customer set a new password, restarting");
  server.send(200, "application/json", "{\"ok\":true}");
  restartAt = millis() + 1500;
}

static void handleResetPass() {
  prefs.remove("appw");
  Serial.println("[wifi] password reset to card password, restarting");
  server.send(200, "application/json", "{\"ok\":true}");
  restartAt = millis() + 1500;
}

static void handleRelease() {
  releaseClient(server.client().remoteIP());
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================ events / loop =================================
static void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  (void)info;
  if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
    Serial.println("[wifi] someone joined");
    blipRequest = true;
  } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
    Serial.println("[wifi] someone left");
  }
}

static void bootButtonTick() {               // hold BOOT 5 s -> forget custom password
  static uint32_t downAt = 0;
  static bool     fired  = false;
  if (digitalRead(BOOT_BTN) != LOW) { downAt = 0; fired = false; return; }
  if (!downAt) { downAt = millis(); return; }
  if (!fired && millis() - downAt >= 5000) {
    fired = true;
    prefs.remove("appw");
    Serial.println("[reset] custom password cleared via BOOT button, restarting");
    for (int i = 0; i < 3; i++) { ledWrite(255, 255, 255); delay(120); ledWrite(0, 0, 0); delay(120); }
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  ledInit();

  prefs.begin("wri", false);
  loadCreds();

  WiFi.persistent(false);
  WiFi.onEvent(onWifiEvent);
  WiFi.mode(WIFI_AP);
  WiFi.softAPsetHostname(HOSTNAME);
  WiFi.softAPConfig(portalIp, portalIp, AP_MASK);
  if (!WiFi.softAP(apSsid.c_str(), apPass.c_str(), AP_CHANNEL, 0, AP_MAX_CLIENTS)) {
    Serial.println("[wifi] softAP failed — check password length / AP_MAX_CLIENTS");
  }
  delay(100);
  portalIp = WiFi.softAPIP();                // whatever actually stuck, use that everywhere

  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
  dns.start(53, "*", portalIp);              // every DNS question -> us

  server.on("/",              HTTP_GET,  handleRoot);
  server.on("/api/status",    HTTP_GET,  handleStatus);
  server.on("/api/led",       HTTP_POST, handleLed);
  server.on("/api/setpass",   HTTP_POST, handleSetPass);
  server.on("/api/resetpass", HTTP_POST, handleResetPass);
  server.on("/api/release",   HTTP_POST, handleRelease);

  static const char *probes[] = {
    "/generate_204", "/gen_204",                                   // android, chrome
    "/hotspot-detect.html", "/library/test/success.html",         // apple
    "/connecttest.txt", "/ncsi.txt", "/redirect", "/fwlink",       // windows
    "/canonical.html", "/success.txt",                             // firefox
    "/chat", "/check_network_status.txt", "/mobile/status.php"     // misc vendors
  };
  for (const char *p : probes) server.on(p, handleProbe);
  server.onNotFound(handleNotFound);
  server.begin();

  ledSetSolid(255, 176, 32);                 // brand amber: "i'm on, come find me"
  delay(1200);                               // give a freshly opened serial monitor a beat
  printBanner();
}

void loop() {
  dns.processNextRequest();                  // no-op on core 3.x (async), harmless
  server.handleClient();
  ledTick();
  bootButtonTick();

  if (Serial.available()) {                  // any keypress reprints the card label
    while (Serial.available()) Serial.read();
    printBanner();
  }
  if (restartAt && (int32_t)(millis() - restartAt) >= 0) ESP.restart();
  delay(2);                                  // single core: let wifi/tcp breathe
}
