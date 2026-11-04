// Bai 1 - BAI TAP: so sanh 3 cach tao nhip dinh ky: delay(), millis(), timer phan cung.
// Chay lan luot 3 pha voi CUNG mot tai "CPU ban" (0..30 ms moi vong), moi pha SO_MAU mau,
// roi in bang so sanh do lech TB / max so voi chu ky dat. Lap lai lien tuc.
// Nut GPIO4: bat/tat CPU ban (bang ket qua se reset va do lai).

#include "TH2Common.h"

static const uint32_t SO_MAU = 50;

ThongKe tkDelay(timerMs * 1000UL);
ThongKe tkMillis(timerMs * 1000UL);
ThongKe tkHw(timerMs * 1000UL);

enum Pha { PHA_DELAY, PHA_MILLIS, PHA_HW, PHA_IN_KQ };
Pha pha = PHA_DELAY;

// ---------- millis ----------
uint32_t loopTick = 0;
void updateLoopTimer() {
  if (millis() - loopTick < timerMs) return;
  loopTick += timerMs;
  if (millis() - loopTick >= timerMs) loopTick = millis();
  tkMillis.ghiNhan(micros());
  doiLed();
  inMau(tkMillis);
}

// ---------- hw timer ----------
hw_timer_t* hwTimer = nullptr;
static const uint8_t KT_HANG_DOI = 64;
volatile uint32_t hangDoi[KT_HANG_DOI];
volatile uint8_t dau = 0, cuoi = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  uint32_t now = (uint32_t)esp_timer_get_time();
  portENTER_CRITICAL_ISR(&mux);
  uint8_t tiep = (dau + 1) & (KT_HANG_DOI - 1);
  if (tiep != cuoi) {
    hangDoi[dau] = now;
    dau = tiep;
  }
  portEXIT_CRITICAL_ISR(&mux);
}

void batTimer() {
  portENTER_CRITICAL(&mux);
  dau = cuoi = 0;
  portEXIT_CRITICAL(&mux);
  batTimerHw(hwTimer);
}

void tatTimer() { tatTimerHw(hwTimer); }

void rutHangDoi() {
  while (true) {
    uint32_t t;
    portENTER_CRITICAL(&mux);
    if (cuoi == dau) {
      portEXIT_CRITICAL(&mux);
      break;
    }
    t = hangDoi[cuoi];
    cuoi = (cuoi + 1) & (KT_HANG_DOI - 1);
    portEXIT_CRITICAL(&mux);
    tkHw.ghiNhan(t);
    doiLed();
    inMau(tkHw);
  }
}

// ---------- dieu khien pha ----------
void batDauLai() {
  tatTimer();
  tkDelay.reset();
  tkMillis.reset();
  tkHw.reset();
  pha = PHA_DELAY;
  Serial.println("\n--- Pha 1/3: delay() ---");
}

void inDong(const char* ten, const ThongKe& tk) {
  Serial.printf("| %-10s | %4u | %12.1f | %13u |\n", ten, tk.soMau, tk.lechTB(), tk.lechMax);
}

void inBangKetQua() {
  Serial.println();
  Serial.printf("===== KET QUA: chu ky %u ms, CPU ban: %s =====\n", timerMs, cpuBan ? "BAT (0..30 ms)" : "TAT");
  Serial.println("| Cach       |  Mau | Lech TB (us) | Lech max (us) |");
  Serial.println("|------------|------|--------------|---------------|");
  inDong("delay()", tkDelay);
  inDong("millis()", tkMillis);
  inDong("hw timer", tkHw);
  Serial.println("==================================================");
  Serial.printf(">tb_delay_us:%.1f\n>tb_millis_us:%.1f\n>tb_hw_us:%.1f\n",
                tkDelay.lechTB(), tkMillis.lechTB(), tkHw.lechTB());
}

void setup() {
  khoiTaoChung();
  Serial.println("\n[Bai1-bai tap] So sanh delay() / millis() / timer phan cung");
  hwTimer = taoTimerHw(timerMs * 1000, &onTimer);  // 1 tick = 1 us
  batDauLai();  // tat timer, cho den pha 3 moi bat
}

void loop() {
  if (kiemTraNut()) {
    batDauLai();
    return;
  }

  switch (pha) {
    case PHA_DELAY:
      tkDelay.ghiNhan(micros());
      doiLed();
      inMau(tkDelay);
      lamViecBan();
      delay(timerMs);
      if (tkDelay.soMau >= SO_MAU) {
        pha = PHA_MILLIS;
        loopTick = millis();
        Serial.println("\n--- Pha 2/3: millis() ---");
      }
      break;

    case PHA_MILLIS:
      updateLoopTimer();
      lamViecBan();
      if (tkMillis.soMau >= SO_MAU) {
        pha = PHA_HW;
        Serial.println("\n--- Pha 3/3: timer phan cung ---");
        batTimer();
      }
      break;

    case PHA_HW:
      rutHangDoi();
      lamViecBan();
      if (tkHw.soMau >= SO_MAU) {
        tatTimer();
        pha = PHA_IN_KQ;
      }
      break;

    case PHA_IN_KQ:
      inBangKetQua();
      delay(3000);
      batDauLai();
      break;
  }
}
