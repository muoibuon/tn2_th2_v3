// Bai 2 - Vi du 1: bien GIU va bien MAT qua deep sleep
// Thuc 5 s (LED nhay) -> deep sleep 10 s -> thuc lai = KHOI DONG LAI tu setup().
//  - soLanThuc (RTC_DATA_ATTR, vung nho RTC) : cong don 1, 2, 3, ...
//  - demThuong (RAM thuong)                  : lan nao cung in ra 0
// Mach: dung chung so do TH1 (LED GPIO2).

#include <Arduino.h>
#include "esp_sleep.h"

#define LED_PIN 2

const uint64_t THOI_GIAN_NGU_US = 10ULL * 1000000ULL;  // 10 giay
const uint32_t THOI_GIAN_THUC_MS = 5000;

// GIU qua deep sleep (vung nho RTC):
RTC_DATA_ATTR int soLanThuc = 0;
// MAT qua deep sleep (RAM thuong):
int demThuong = 0;

const char* tenNguyenNhan(esp_sleep_wakeup_cause_t c) {
  switch (c) {
    case ESP_SLEEP_WAKEUP_TIMER:     return "ESP_SLEEP_WAKEUP_TIMER";
    case ESP_SLEEP_WAKEUP_EXT0:      return "ESP_SLEEP_WAKEUP_EXT0";
    case ESP_SLEEP_WAKEUP_UNDEFINED: return "UNDEFINED (cap nguon / nhan EN)";
    default:                         return "KHAC";
  }
}

void lamDeepSleep() {
  soLanThuc++;  // RTC: cong don
  demThuong++;  // thuong: se mat
  Serial.printf("Truoc khi ngu: soLanThuc=%d, demThuong=%d -> deep sleep 10 s\n", soLanThuc, demThuong);
  Serial.flush();
  digitalWrite(LED_PIN, LOW);  // tat LED
  esp_sleep_enable_timer_wakeup(THOI_GIAN_NGU_US);
  esp_deep_sleep_start();
  // khong bao gio toi day
}

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n\n########## setup() - chip vua khoi dong ##########");
  Serial.printf("Nguyen nhan thuc : %s\n", tenNguyenNhan(esp_sleep_get_wakeup_cause()));
  Serial.printf("soLanThuc (RTC)  : %d\n", soLanThuc);
  Serial.printf("demThuong (RAM)  : %d\n", demThuong);
  Serial.printf(">soLanThuc:%d\n>demThuong:%d\n", soLanThuc, demThuong);
  Serial.printf("Thuc %u ms (LED nhay) roi ngu...\n", THOI_GIAN_THUC_MS);
}

void loop() {
  static uint32_t lanNhay = 0;
  static bool led = false;
  if (millis() - lanNhay >= 200) {  // LED nhap nhay bao CPU con chay
    lanNhay = millis();
    led = !led;
    digitalWrite(LED_PIN, led);
  }
  if (millis() >= THOI_GIAN_THUC_MS) lamDeepSleep();
}
