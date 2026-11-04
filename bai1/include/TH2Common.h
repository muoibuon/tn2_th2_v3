// Dung chung cho Bai 1 (bai tap): thong ke jitter, gia lap "CPU ban", nut bat/tat, LED.
#pragma once
#include <Arduino.h>

#define LED_PIN 2
#define NUT_PIN 4

// Chu ky muc tieu cua moi cach tao nhip
static const uint32_t timerMs = 100;

// ---------------- Thong ke khoang thoi gian giua 2 su kien ----------------
struct ThongKe {
  uint32_t chuKyUs;
  uint32_t lanTruoc = 0;
  bool     coLanTruoc = false;
  uint32_t soMau = 0;
  uint64_t tongLech = 0;   // tong |khoang thuc - chu ky| (us)
  uint32_t lechMax = 0;    // |lech| lon nhat (us)
  uint32_t khoangCuoi = 0; // khoang gan nhat (us)
  int32_t  lechCuoi = 0;   // lech gan nhat (us), co dau

  explicit ThongKe(uint32_t ckUs) : chuKyUs(ckUs) {}

  void ghiNhan(uint32_t now) {
    if (coLanTruoc) {
      khoangCuoi = now - lanTruoc;
      lechCuoi = (int32_t)khoangCuoi - (int32_t)chuKyUs;
      uint32_t a = (uint32_t)(lechCuoi < 0 ? -lechCuoi : lechCuoi);
      tongLech += a;
      if (a > lechMax) lechMax = a;
      soMau++;
    }
    lanTruoc = now;
    coLanTruoc = true;
  }

  float lechTB() const { return soMau ? (float)tongLech / soMau : 0.0f; }

  void reset() {
    coLanTruoc = false;
    soMau = 0;
    tongLech = 0;
    lechMax = 0;
  }
};

// ---------------- Gia lap CPU ban ----------------
static bool cpuBan = true;

// Ban CPU ngau nhien 0..maxMs (busy-wait, khong nhuong CPU)
inline void lamViecBan(uint32_t maxMs = 30) {
  if (!cpuBan) return;
  uint32_t us = (uint32_t)random(0, maxMs + 1) * 1000UL;
  uint32_t t = micros();
  while (micros() - t < us) { /* ban */ }
}

// ---------------- Nut GPIO4: bat/tat CPU ban ----------------
static bool nutTruoc = HIGH;
static uint32_t lanBamNut = 0;

inline void khoiTaoChung() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(NUT_PIN, INPUT_PULLUP);
  randomSeed(esp_random());
}

// Tra ve true neu vua doi trang thai CPU ban
inline bool kiemTraNut() {
  bool n = digitalRead(NUT_PIN);
  bool doi = false;
  if (n == LOW && nutTruoc == HIGH && millis() - lanBamNut > 200) {
    cpuBan = !cpuBan;
    lanBamNut = millis();
    Serial.printf("\n*** CPU ban: %s ***\n", cpuBan ? "BAT" : "TAT");
    doi = true;
  }
  nutTruoc = n;
  return doi;
}

// ---------------- LED ----------------
static bool trangThaiLed = false;
inline void doiLed() {
  trangThaiLed = !trangThaiLed;
  digitalWrite(LED_PIN, trangThaiLed);
}

// ---------------- Timer phan cung: tuong thich core 2.x va 3.x ----------------
// Core 2.x (PlatformIO espressif32@6.x): timerBegin(so_timer, he_so_chia, dem_len) - nhu slide
// Core 3.x (wokwi.com, Arduino IDE moi):  timerBegin(tan_so_Hz)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#define TH2_CORE3 1
#endif

inline hw_timer_t* taoTimerHw(uint32_t chuKyUs, void (*isr)()) {
#ifdef TH2_CORE3
  hw_timer_t* t = timerBegin(1000000);       // 1 MHz -> 1 tick = 1 us
  timerAttachInterrupt(t, isr);
  timerAlarm(t, chuKyUs, true, 0);           // tu nap lai
#else
  hw_timer_t* t = timerBegin(0, 80, true);   // timer0, chia 80 -> 1 tick = 1 us
  timerAttachInterrupt(t, isr, true);
  timerAlarmWrite(t, chuKyUs, true);         // tu nap lai
  timerAlarmEnable(t);
#endif
  return t;
}

inline void batTimerHw(hw_timer_t* t) {
  timerWrite(t, 0);
#ifdef TH2_CORE3
  timerStart(t);
#else
  timerAlarmEnable(t);
#endif
}

inline void tatTimerHw(hw_timer_t* t) {
#ifdef TH2_CORE3
  timerStop(t);
#else
  timerAlarmDisable(t);
#endif
}

// ---------------- In ket qua ----------------
inline void inMau(const ThongKe& tk) {
  if (tk.soMau == 0) return;
  Serial.printf(">khoang_us:%u\n>lech_us:%d\n", tk.khoangCuoi, tk.lechCuoi);
}

inline void inTongKet(const char* ten, const ThongKe& tk) {
  static uint32_t lanIn = 0;
  if (millis() - lanIn < 2000) return;
  lanIn = millis();
  Serial.printf("[%s] chu ky=%u ms | CPU ban=%s | mau=%u | lech TB=%.1f us | lech max=%u us\n",
                ten, timerMs, cpuBan ? "BAT" : "TAT", tk.soMau, tk.lechTB(), tk.lechMax);
  Serial.printf(">lechTB_us:%.1f\n>lechMax_us:%u\n", tk.lechTB(), tk.lechMax);
}
