#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("TFT screen test");

  tft.init(135, 240);
  tft.setRotation(3);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Hello, World!");

  tft.drawRect(10, 50, 100, 50, ST77XX_RED);
  tft.fillCircle(60, 120, 30, ST77XX_BLUE);
}

void loop() {
}