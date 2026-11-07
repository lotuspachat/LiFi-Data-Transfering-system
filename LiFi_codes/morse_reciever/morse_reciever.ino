#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);   // RS, E, D4, D5, D6, D7

const int THRESHOLD = 450;        // below this = flashlight on
const unsigned long UNIT = 400;   // measured: dot ≈ 475, gap ≈ 342

// Morse patterns for A–Z and 0–9
const char* letters[] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",
  ".---","-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-","..-",
  "...-",".--","-..-","-.--","--.."};
const char* digits[] = {"-----",".----","..---","...--","....-",
  ".....","-....","--...","---..","----."};

String symbol = "";   // dots/dashes of the letter being received
String text = "";     // the decoded message
bool lightOn = false;
unsigned long changeTime = 0;
bool letterDone = true, wordDone = true;

// Turn a pattern like ".-" into a character like 'A'
char decode(String s) {
  for (int i = 0; i < 26; i++) if (s == letters[i]) return 'A' + i;
  for (int i = 0; i < 10; i++) if (s == digits[i]) return '0' + i;
  return '?';                     // pattern not recognised
}

// Add a character to the message and redraw the LCD
void show(char c) {
  text += c;
  if (text.length() > 32) text = text.substring(text.length() - 32); // keep last 32
  lcd.clear();
  lcd.print(text.substring(0, min(16, (int)text.length())));        // row 1
  if (text.length() > 16) { lcd.setCursor(0, 1); lcd.print(text.substring(16)); } // row 2
  Serial.print(c);
}

void setup() {
  pinMode(10, OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("Waiting...");
}

void loop() {
  bool now = analogRead(A0) < THRESHOLD;   // true = flashlight on
  unsigned long t = millis();
  digitalWrite(10, now);                   // data LED (if fitted) copies the light

  if (now != lightOn) {                    // light just changed
    unsigned long dur = t - changeTime;
    if (lightOn && dur > UNIT / 3) {       // a flash just ended (ignore tiny glitches)
      symbol += (dur < 2 * UNIT) ? '.' : '-';
      letterDone = false; wordDone = false;
    }
    lightOn = now;
    changeTime = t;
  }

  if (!lightOn) {                          // measure the dark gap
    unsigned long gap = t - changeTime;
    if (!letterDone && gap > 2 * UNIT) {   // end of letter
      show(decode(symbol)); symbol = ""; letterDone = true;
    }
    if (!wordDone && gap > 5 * UNIT) {     // end of word
      show(' '); wordDone = true;
    }
  }
}