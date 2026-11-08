#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);   // RS, E, D4, D5, D6, D7

void setup() {
  pinMode(2, INPUT);       // LM393 output (pin 1) → D2
  lcd.begin(16, 2);        // 16 columns, 2 rows
  Serial.begin(9600);      // also send readings to the Serial Monitor
}

void loop() {
  int raw = analogRead(A0);   // LDR node X: 0–1023, goes UP with more light
  int bit = digitalRead(2);   // comparator decision: 1 = light, 0 = dark

  lcd.setCursor(0, 0);
  lcd.print("LDR:"); lcd.print(raw); lcd.print("    ");  // spaces clear old digits
  lcd.setCursor(0, 1);
  lcd.print("BIT:"); lcd.print(bit);

  Serial.print(raw); Serial.print("  "); Serial.println(bit);
  delay(100);                 // update 10 times a second
}
