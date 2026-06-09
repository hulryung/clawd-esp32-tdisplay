#include <Wire.h>

// Integrated ESP32+OLED boards (Heltec/WeMos/TTGO) usually wire the OLED
// reset to a GPIO that must be pulsed low->high before I2C works.
// We try several (SDA, SCL, RST) combos used by these boards.
struct Combo { int sda; int scl; int rst; const char* name; };
Combo combos[] = {
  { 4, 15, 16, "Heltec WiFi Kit 32"},
  { 5,  4, 16, "TTGO/LilyGO"},
  {21, 22, 16, "dev+rst16"},
  {21, 22, -1, "dev no-rst"},
  { 4, 15, -1, "4/15 no-rst"},
  {17, 18, 21, "alt"},
};

void pulseRst(int rst) {
  if (rst < 0) return;
  pinMode(rst, OUTPUT);
  digitalWrite(rst, LOW);
  delay(50);
  digitalWrite(rst, HIGH);
  delay(50);
}

void scan(Combo &c) {
  pulseRst(c.rst);
  Wire.end();
  delay(20);
  if (!Wire.begin(c.sda, c.scl, 100000)) {
    Serial.printf("SDA=%2d SCL=%2d RST=%2d (%-20s): begin failed\n", c.sda, c.scl, c.rst, c.name);
    return;
  }
  delay(20);
  int found = 0; String hits = "";
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found++;
      char b[8]; snprintf(b, sizeof(b), "0x%02X ", addr); hits += b;
    }
  }
  Serial.printf("SDA=%2d SCL=%2d RST=%2d (%-20s): %d found  %s\n",
                c.sda, c.scl, c.rst, c.name, found, hits.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== I2C scanner (with reset handling) ===");
}

void loop() {
  for (auto &c : combos) scan(c);
  Serial.println("--- pass done ---\n");
  delay(3000);
}
