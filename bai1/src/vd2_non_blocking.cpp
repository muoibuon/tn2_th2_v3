// Bai 1 - Vi du 2: NON-BLOCKING bang millis()
// Mach: dung chung so do TH1 (LED1 -> GPIO2, bien tro -> GPIO34)
//
// Moi viec tu so sanh millis() voi moc lan truoc cua minh, khong co delay() nao.
// loop() chay lai lien tuc, moi viec chi "lam that" dung luc -> khoang cach deu.

#include <Arduino.h>

#define LED1    2
#define ADC_PIN 34

const unsigned long CK_LED   = 500;
const unsigned long CK_ADC   = 2000;
const unsigned long CK_HEART = 5000;

unsigned long lanTruocLed = 0, lanTruocAdc = 0, lanTruocHeart = 0;
bool trangThaiLed = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  analogReadResolution(12);
  Serial.println("\n[Bai1-vd2] Non-blocking bang millis()");
  unsigned long now = millis();
  lanTruocLed = lanTruocAdc = lanTruocHeart = now;
}

void loop() {
  unsigned long now = millis();

  // Viec 1: nhay LED moi 500 ms
  if (now - lanTruocLed >= CK_LED) {
    Serial.printf(">khoangLed_ms:%lu\n", now - lanTruocLed);
    lanTruocLed = now;
    trangThaiLed = !trangThaiLed;
    digitalWrite(LED1, trangThaiLed);
    Serial.printf(">led:%d\n", trangThaiLed);
  }

  // Viec 2: doc bien tro moi 2000 ms
  if (now - lanTruocAdc >= CK_ADC) {
    Serial.printf(">khoangAdc_ms:%lu\n", now - lanTruocAdc);
    lanTruocAdc = now;
    Serial.printf(">bienTro:%d\n", analogRead(ADC_PIN));
  }

  // Viec 3: heartbeat moi 5000 ms
  if (now - lanTruocHeart >= CK_HEART) {
    Serial.printf(">khoangHeart_ms:%lu\n", now - lanTruocHeart);
    lanTruocHeart = now;
    Serial.println("heartbeat");
  }
}
