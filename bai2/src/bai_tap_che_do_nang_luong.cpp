// Bai 2 - BAI TAP: so sanh 4 muc tieu thu dien, quan sat GIAN TIEP qua hanh vi
// OLED SSD1306 la TUY CHON: khong cam thi chi quan sat qua LED, Serial va web.
//
// Bam nut GPIO4 de chuyen che do:  ACTIVE -> MODEM-SLEEP -> LIGHT-SLEEP -> DEEP-SLEEP
//  1. ACTIVE      : CPU 240 MHz, Wi-Fi AP "TH2-ESP32" (mk 12345678) + web http://192.168.4.1
//                   LED nhay nhanh, web tu cap nhat moi 1 s, loop chay hang tram nghin vong/s.
//  2. MODEM-SLEEP : tat RF (Wi-Fi off), CPU ha 80 MHz, CPU van chay. LED nhay cham.
//                   Dau hieu: mat Wi-Fi / web khong vao duoc, nhung Serial + OLED van cap nhat.
//  3. LIGHT-SLEEP : CPU dung, ngu 2 s roi thuc lai NGAY TAI CHO goi ham. LED chop ngan moi 2 s.
//                   Dau hieu: vong loop/s ~0.5, bien RAM thuong (soLanLight) van tang dan.
//  4. DEEP-SLEEP  : tat gan het, OLED tat. Thuc sau 10 s hoac bam nut -> chay lai tu setup().
//                   Dau hieu: Serial in lai banner khoi dong, chi bien RTC_DATA_ATTR con giu.
//
// Chu y: o day "modem-sleep" duoc minh hoa bang cach tat han RF (WIFI_OFF). Modem-sleep "chuan"
// cua ESP-IDF can ESP32 ket noi STA toi 1 AP (RF tat/bat theo chu ky beacon).

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

#define LED_PIN 2
#define NUT_PIN 4
#define ADC_PIN 34

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool coOled = false;
WebServer server(80);

enum CheDo { ACTIVE = 0, MODEM_SLEEP, LIGHT_SLEEP, DEEP_SLEEP, SO_CHE_DO };
const char* TEN_CHE_DO[] = {"ACTIVE", "MODEM-SLEEP", "LIGHT-SLEEP", "DEEP-SLEEP"};

RTC_DATA_ATTR uint32_t soLanDeep = 0;  // giu qua deep sleep
uint32_t soLanLight = 0;               // RAM thuong: giu qua light sleep, mat qua deep sleep

int cheDo = ACTIVE;
uint32_t batDauCheDo = 0;
uint32_t demVong = 0, vongMoiGiay = 0, lanTinhVong = 0;
uint32_t lanNhayLed = 0;
bool led = false;
uint32_t nguLightCuoiMs = 0;

// ---------------- OLED ----------------
void hienThi(const char* dong3 = "", const char* dong4 = "") {
  if (!coOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.printf("Che do %d/4:", cheDo + 1);
  oled.setTextSize(cheDo == ACTIVE ? 2 : 1);
  oled.setCursor(0, 12);
  oled.print(TEN_CHE_DO[cheDo]);
  oled.setTextSize(1);
  oled.setCursor(0, 34);
  oled.print(dong3);
  oled.setCursor(0, 46);
  oled.print(dong4);
  oled.setCursor(0, 56);
  oled.printf("deep:%u light:%u", soLanDeep, soLanLight);
  oled.display();
}

// ---------------- Web (chi o ACTIVE) ----------------
void handleRoot() {
  String html =
      "<!doctype html><meta charset='utf-8'><meta http-equiv='refresh' content='1'>"
      "<meta name='viewport' content='width=device-width'>"
      "<body style='font-family:sans-serif'><h2>TH2 - Bai 2 (bai tap)</h2>";
  html += "<p>Che do: <b>" + String(TEN_CHE_DO[cheDo]) + "</b></p>";
  html += "<p>Thoi gian chay: " + String(millis() / 1000) + " s</p>";
  html += "<p>Vong loop/s: " + String(vongMoiGiay) + "</p>";
  html += "<p>CPU: " + String(getCpuFrequencyMhz()) + " MHz</p>";
  html += "<p>So lan deep sleep (RTC_DATA_ATTR): " + String(soLanDeep) + "</p>";
  html += "<p>Bien tro: " + String(analogRead(ADC_PIN)) + "</p>";
  html += "<p><i>Bam nut GPIO4 tren mach de chuyen sang MODEM-SLEEP: trang nay se mat ket noi.</i></p>";
  server.send(200, "text/html", html);
}

// ---------------- Nut ----------------
bool nutTruoc = HIGH;
uint32_t lanBamNut = 0;
bool nutVuaBam() {
  bool n = digitalRead(NUT_PIN);
  bool bam = (n == LOW && nutTruoc == HIGH && millis() - lanBamNut > 250);
  if (bam) lanBamNut = millis();
  nutTruoc = n;
  return bam;
}

void choNhaNut() {
  while (digitalRead(NUT_PIN) == LOW) delay(10);
  delay(50);
  nutTruoc = HIGH;
}

// ---------------- Deep sleep ----------------
void lamDeepSleep() {
  hienThi("Ngu 10 s...", "Bam nut de thuc");
  Serial.println("-> DEEP SLEEP 10 s (hoac bam nut GPIO4). Khi thuc se chay lai tu setup().");
  choNhaNut();  // tranh ext0 muc thap lam thuc ngay
  delay(1500);  // cho nguoi xem kip doc OLED
  if (coOled) oled.ssd1306_command(SSD1306_DISPLAYOFF);
  digitalWrite(LED_PIN, LOW);
  soLanDeep++;

  esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
  rtc_gpio_pullup_en(GPIO_NUM_4);
  rtc_gpio_pulldown_dis(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);  // nut keo xuong GND -> muc 0
  Serial.flush();
  esp_deep_sleep_start();
}

// ---------------- Chuyen che do ----------------
void vaoCheDo(int m) {
  cheDo = m;
  batDauCheDo = millis();
  Serial.printf("\n===== CHE DO %d/4: %s =====\n", cheDo + 1, TEN_CHE_DO[cheDo]);
  Serial.printf(">cheDo:%d\n", cheDo);

  switch (cheDo) {
    case ACTIVE:
      setCpuFrequencyMhz(240);
#ifdef WOKWI
      // Wokwi: chi ho tro STA, mang ao "Wokwi-GUEST" (khong mat khau, kenh 6)
      WiFi.mode(WIFI_STA);
      WiFi.begin("Wokwi-GUEST", "", 6);
      Serial.print("Ket noi Wokwi-GUEST");
      for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) { delay(250); Serial.print("."); }
      Serial.println();
      server.on("/", handleRoot);
      server.begin();
      Serial.printf("IP: %s -> mo http://localhost:8180 tren may tinh\n", WiFi.localIP().toString().c_str());
#else
      WiFi.mode(WIFI_AP);
      WiFi.softAP("TH2-ESP32", "12345678");
      server.on("/", handleRoot);
      server.begin();
      Serial.printf("Wi-Fi AP: TH2-ESP32 / 12345678 -> http://%s\n", WiFi.softAPIP().toString().c_str());
#endif
      break;

    case MODEM_SLEEP:
      server.stop();
#ifdef WOKWI
      WiFi.disconnect(true);
#else
      WiFi.softAPdisconnect(true);
#endif
      WiFi.mode(WIFI_OFF);  // tat RF
      setCpuFrequencyMhz(80);
      Serial.println("Da tat Wi-Fi, CPU 80 MHz. Web se mat ket noi.");
      break;

    case LIGHT_SLEEP:
      Serial.println("Ngu light 2 s moi chu ky. Bam nut de sang DEEP-SLEEP.");
      break;

    case DEEP_SLEEP:
      lamDeepSleep();
      break;
  }
  hienThi();
}

// ---------------- Light sleep ----------------
void chuKyLightSleep() {
  // Chop LED ngan bao "dang thuc"
  digitalWrite(LED_PIN, HIGH);
  delay(30);
  digitalWrite(LED_PIN, LOW);

  char d3[24], d4[24];
  snprintf(d3, sizeof(d3), "Ngu 2s, lan %u", soLanLight);
  snprintf(d4, sizeof(d4), "ngu that: %u ms", nguLightCuoiMs);
  hienThi(d3, d4);

  esp_sleep_enable_timer_wakeup(2ULL * 1000000ULL);
  gpio_wakeup_enable(GPIO_NUM_4, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  Serial.flush();

  int64_t t0 = esp_timer_get_time();
  esp_light_sleep_start();  // CPU dung o day ...
  int64_t t1 = esp_timer_get_time();  // ... va chay tiep ngay dong nay khi thuc

  gpio_wakeup_disable(GPIO_NUM_4);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  soLanLight++;
  nguLightCuoiMs = (uint32_t)((t1 - t0) / 1000);

  esp_sleep_wakeup_cause_t c = esp_sleep_get_wakeup_cause();
  Serial.printf("Thuc tu light sleep (%s) sau %u ms, soLanLight=%u, millis=%lu\n",
                c == ESP_SLEEP_WAKEUP_GPIO ? "NUT" : "TIMER", nguLightCuoiMs, soLanLight, millis());
  Serial.printf(">nguLight_ms:%u\n>soLanLight:%u\n", nguLightCuoiMs, soLanLight);

  if (c == ESP_SLEEP_WAKEUP_GPIO) {
    choNhaNut();
    vaoCheDo(DEEP_SLEEP);
  }
}

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);
  delay(200);
  rtc_gpio_deinit(GPIO_NUM_4);  // tra GPIO4 ve GPIO thuong sau ext0
  pinMode(LED_PIN, OUTPUT);
  pinMode(NUT_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  Wire.begin(21, 22);
  coOled = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (coOled) oled.ssd1306_command(SSD1306_DISPLAYON);
  else Serial.println("!! Khong tim thay OLED o dia chi 0x3C (kiem tra SDA=21, SCL=22)");

  esp_sleep_wakeup_cause_t c = esp_sleep_get_wakeup_cause();
  Serial.println("\n\n########## KHOI DONG (setup) ##########");
  Serial.printf("Nguyen nhan thuc: %s | soLanDeep (RTC)=%u | soLanLight (RAM)=%u\n",
                c == ESP_SLEEP_WAKEUP_TIMER ? "TIMER" :
                c == ESP_SLEEP_WAKEUP_EXT0  ? "EXT0 (nut)" : "CAP NGUON / RESET",
                soLanDeep, soLanLight);

  vaoCheDo(ACTIVE);
}

void loop() {
  demVong++;

  if (cheDo != LIGHT_SLEEP && nutVuaBam()) {
    vaoCheDo((cheDo + 1) % SO_CHE_DO);
    return;
  }

  switch (cheDo) {
    case ACTIVE:
      server.handleClient();
      if (millis() - lanNhayLed >= 100) { lanNhayLed = millis(); led = !led; digitalWrite(LED_PIN, led); }
      break;
    case MODEM_SLEEP:
      if (millis() - lanNhayLed >= 500) { lanNhayLed = millis(); led = !led; digitalWrite(LED_PIN, led); }
      break;
    case LIGHT_SLEEP:
      chuKyLightSleep();
      break;
  }

  // Moi 1 s: so vong loop/s - dau hieu CPU "ranh" bao nhieu
  if (millis() - lanTinhVong >= 1000) {
    vongMoiGiay = demVong;
    demVong = 0;
    lanTinhVong = millis();
    Serial.printf(">vongLoop_per_s:%u\n>cpu_MHz:%u\n>cheDo:%d\n", vongMoiGiay, getCpuFrequencyMhz(), cheDo);
    if (cheDo != LIGHT_SLEEP) {
      char d3[24], d4[24];
      snprintf(d3, sizeof(d3), "CPU %u MHz", getCpuFrequencyMhz());
      snprintf(d4, sizeof(d4), "loop/s: %u", vongMoiGiay);
      hienThi(d3, d4);
    }
  }
}
