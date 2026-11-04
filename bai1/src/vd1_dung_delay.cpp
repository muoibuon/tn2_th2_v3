// Bai 1 - Vi du 1: 3 viec viet NOI TIEP bang delay()
// Mach: dung chung so do TH1 (LED1 -> GPIO2, bien tro -> GPIO34)
//
// Mong muon: LED nhay moi 500 ms, doc bien tro moi 2000 ms, heartbeat moi 5000 ms.
// Thuc te: 1 vong loop = 500 + 500 + 2000 + 5000 = 8000 ms -> moi viec chi chay 1 lan / 8 s.
// Cac dong ">ten:gia_tri" la dinh dang Teleplot de plot_serial_th2.py ve do thi.

#include <Arduino.h>

#define LED1    2
#define ADC_PIN 34

unsigned long lanTruocLed = 0, lanTruocAdc = 0, lanTruocHeart = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  analogReadResolution(12);  // 0..4095
  Serial.println("\n[Bai1-vd1] Viet noi tiep bang delay()");
}

void loop() {
  unsigned long now = millis();
  Serial.printf(">khoangLed_ms:%lu\n", now - lanTruocLed);  // ly tuong 500
  lanTruocLed = now;

  digitalWrite(LED1, HIGH);
  Serial.println(">led:1");
  delay(500);
  digitalWrite(LED1, LOW);
  Serial.println(">led:0");
  delay(500);

  now = millis();
  int tho = analogRead(ADC_PIN);
  Serial.printf(">bienTro:%d\n", tho);
  Serial.printf(">khoangAdc_ms:%lu\n", now - lanTruocAdc);  // ly tuong 2000
  lanTruocAdc = now;
  delay(2000);

  now = millis();
  Serial.println("heartbeat");
  Serial.printf(">khoangHeart_ms:%lu\n", now - lanTruocHeart);  // ly tuong 5000
  lanTruocHeart = now;
  delay(5000);
}
