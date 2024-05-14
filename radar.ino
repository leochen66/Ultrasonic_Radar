#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

#define TFT_CS   15
#define TFT_DC   33
#define TFT_RST -1

int width = 480;
int height = 320;
uint16_t black = 0x0000;
uint16_t green = 0x07E0;

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(250);
  tft.begin();
  delay(1000);
  tft.fillScreen(black);
  tft.setRotation(1);
  tft.setCursor(0,0);

  tft.fillScreen(black);

  for (int r = 0; r < (width / 2); r += 25) {
    tft.drawCircle(width / 2, 0, r, green);
  }
}

void loop() {
  drawRadar();
}

void drawRadar() {

  int prev_x = 0;
  int prev_y = 0;

  for (int angle = 0; angle < 360; angle += 10) {

    // 清除上一次繪製的直線
    tft.drawLine(width / 2, 0, prev_x, prev_y, black);

    // 計算最外圈同心圓的半徑
    int max_radius = (width / 2) - 25;

    // 計算直線終點位置
    int end_x = width / 2 + cos(PI / 180 * angle) * max_radius;
    int end_y = sin(PI / 180 * angle) * max_radius;

    // 繪製新的直線
    tft.drawLine(width / 2, 0, end_x, end_y, green);

    // 更新上一次線段的終點位置
    prev_x = end_x;
    prev_y = end_y;

    delay(100);
  }
}
