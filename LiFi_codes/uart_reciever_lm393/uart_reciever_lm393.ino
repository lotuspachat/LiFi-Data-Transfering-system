#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);   // RS, E, D4, D5, D6, D7

const unsigned long BIT_MS = 250;       // must match the webpage
const unsigned long NEW_MSG_GAP = 3000; // dark this long (ms) = next character starts a new message

String text = "";
unsigned long lastByteTime = 0;

bool light() { return digitalRead(2); } // LM393 output: HIGH = torch ON

// Busy-wait until 'offset' ms after 'start'
void waitFor(unsigned long start, unsigned long offset) {
  while (millis() - start < offset) {}
}

void showText() {
  if (text.length() > 32) text = text.substring(text.length() - 32); // keep last 32 chars
  lcd.clear();
  lcd.print(text.substring(0, min(16, (int)text.length())));          // row 1
  if (text.length() > 16) { lcd.setCursor(0, 1); lcd.print(text.substring(16)); } // row 2
}

void setup() {
  pinMode(2, INPUT);                        // LM393 output → D2
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("Waiting...");
}

void loop() {
  if (!light()) return;                     // idle: wait for a start bit

  unsigned long start = millis();           // start bit detected: timing reference
  if (start - lastByteTime > NEW_MSG_GAP) text = "";   // long pause before this = new message

  waitFor(start, BIT_MS / 2);               // middle of the start bit
  if (!light()) return;                     // just a flicker, not a start bit

  byte b = 0;
  for (int i = 0; i < 8; i++) {             // read 8 data bits, D0 first
    waitFor(start, BIT_MS / 2 + (i + 1) * BIT_MS);
    if (light()) b |= (1 << i);             // light ON = 1
  }

  waitFor(start, BIT_MS / 2 + 9 * BIT_MS);  // middle of the stop bit
  bool stopOk = !light();                   // stop bit must be dark

  if (stopOk) {
    text += (char)b;
    Serial.print((char)b); Serial.print("  ");
    for (int i = 7; i >= 0; i--) Serial.print((b >> i) & 1);   // show the bits
    Serial.println();
  } else {
    text += '?';                            // framing error
    Serial.println("? framing error");
  }
  showText();
  lastByteTime = millis();

  while (light()) {}                        // if still lit (error case), wait until dark
}