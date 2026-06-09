#include <Arduino_GFX_Library.h>

// TTGO T-Display (ST7789V) pin map — from the board photo
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL    4

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED);
// 240x135 IPS panel with the standard T-Display offsets
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 1 /*rotation*/, true /*IPS*/,
                                      135, 240, 52, 40, 53, 40);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ST7789 T-Display test ===");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // backlight on

  if (!gfx->begin()) {
    Serial.println("gfx->begin() FAILED");
  } else {
    Serial.println("gfx->begin() OK");
  }

  int16_t w = gfx->width(), h = gfx->height();
  Serial.printf("Display size: %d x %d\n", w, h);

  // Color bars to confirm orientation & color order
  gfx->fillScreen(RGB565_BLACK);
  gfx->fillRect(0,         0, w/3,   h, RGB565_RED);
  gfx->fillRect(w/3,       0, w/3,   h, RGB565_GREEN);
  gfx->fillRect(2*(w/3),   0, w-2*(w/3), h, RGB565_BLUE);

  // Border + label
  gfx->drawRect(0, 0, w, h, RGB565_WHITE);
  gfx->setTextColor(RGB565_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(8, 8);
  gfx->println("ST7789 OK");
  gfx->setTextSize(1);
  gfx->setCursor(8, 30);
  gfx->printf("%d x %d", w, h);

  Serial.println("draw done");
}

void loop() {
  delay(1000);
}
