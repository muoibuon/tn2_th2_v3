// Bai 2 - Vi du 2: thuc day = chay lai tu setup(), doc nguyen nhan thuc
// Danh thuc bang TIMER hoac NUT NGOAI (ext0 - GPIO4)
// - Ngu toi da 20 s; bam nut GPIO4 de thuc som.
// - Doc esp_sleep_get_wakeup_cause(): TIMER -> ESP_SLEEP_WAKEUP_TIMER, nut -> ESP_SLEEP_WAKEUP_EXT0
// - Dem rieng so lan thuc theo tung nguyen nhan bang bien RTC_DATA_ATTR.
// Mach: so do TH1. Nut: GPIO4 <-> GND. Nen them dien tro 10k tu GPIO4 len 3V3
// (code cung bat pull-up RTC noi, nhung dien tro ngoai on dinh hon).

#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define LED_PIN 2
#define NUT_PIN 4  // RTC_GPIO10 -> dung duoc cho ext0

const uint64_t THOI_GIAN_NGU_US = 20ULL * 1000000ULL;
const uint32_t THOI_GIAN_THUC_MS = 5000;

RTC_DATA_ATTR int soLanKhoiDong = 0;
RTC_DATA_ATTR int soLanTimer = 0;
RTC_DATA_ATTR int soLanNut = 0;
int demThuong = 0;

void lamDeepSleep() {
  // Neu nut dang bi giu, ext0 muc 0 se lam chip thuc ngay -> cho nha nut
  while (digitalRead(NUT_PIN) == LOW) delay(10);

  Serial.println("-> deep sleep toi da 20 s (bam nut GPIO4 de thuc som)");
  Serial.flush();
  digitalWrite(LED_PIN, LOW);

  esp_sleep_enable_timer_wakeup(THOI_GIAN_NGU_US);
  rtc_gpio_pullup_en(GPIO_NUM_4);  // giu muc 1 khi ngu
  rtc_gpio_pulldown_dis(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);  // thuc khi GPIO4 = 0 (nut bam xuong GND)
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  rtc_gpio_deinit(GPIO_NUM_4);  // tra GPIO4 ve che do GPIO thuong sau khi dung ext0
  pinMode(LED_PIN, OUTPUT);
  pinMode(NUT_PIN, INPUT_PULLUP);

  soLanKhoiDong++;
  esp_sleep_wakeup_cause_t c = esp_sleep_get_wakeup_cause();
  const char* ten;
  switch (c) {
    case ESP_SLEEP_WAKEUP_TIMER: ten = "ESP_SLEEP_WAKEUP_TIMER (het 20 s)"; soLanTimer++; break;
    case ESP_SLEEP_WAKEUP_EXT0:  ten = "ESP_SLEEP_WAKEUP_EXT0 (bam nut GPIO4)"; soLanNut++; break;
    default:                     ten = "UNDEFINED (cap nguon / nhan EN)"; break;
  }

  Serial.println("\n\n########## setup() ##########");
  Serial.printf("Nguyen nhan thuc : %s (ma %d)\n", ten, (int)c);
  Serial.printf("RTC: khoiDong=%d, timer=%d, nut=%d | RAM: demThuong=%d\n",
                soLanKhoiDong, soLanTimer, soLanNut, demThuong);
  Serial.printf(">wakeCause:%d\n>soLanTimer:%d\n>soLanNut:%d\n", (int)c, soLanTimer, soLanNut);

  // LED bao nguyen nhan: nhay 1 lan = timer, 3 lan = nut
  int n = (c == ESP_SLEEP_WAKEUP_EXT0) ? 3 : 1;
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_PIN, HIGH); delay(150);
    digitalWrite(LED_PIN, LOW);  delay(150);
  }
  demThuong++;
}

void loop() {
  static uint32_t lanNhay = 0;
  static bool led = false;
  if (millis() - lanNhay >= 500) {
    lanNhay = millis();
    led = !led;
    digitalWrite(LED_PIN, led);
  }
  if (millis() >= THOI_GIAN_THUC_MS) lamDeepSleep();
}
