#include <SPI.h>
#include <WS2812FX.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

#define NUM_SENSORS 4

#define trigPin1 8
#define echoPin1 7
#define trigPin2 21
#define echoPin2 19
#define trigPin3 5
#define echoPin3 4
#define trigPin4 36
#define echoPin4 39

#define LED_COUNT 50
#define LED_PIN 14

#define TFT_CS   15
#define TFT_DC   33
#define TFT_RST  -1

int width = 480;
int height = 320;
uint16_t black = 0x0000;
uint16_t green = 0x07E0;

const int trigPins[NUM_SENSORS] = {trigPin1, trigPin2, trigPin3, trigPin4};
const int echoPins[NUM_SENSORS] = {echoPin1, echoPin2, echoPin3, echoPin4};

# Screen controller
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

# LED controller
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

 
void setup() {
  Serial.begin(115200);
  delay(250);

  screenSetup();

  screenReset();
  
  // sensorSetup();

}

void loop() {
  drawRadar();
}

def screenSetup() {
  tft.begin();
  delay(1000);
  tft.fillScreen(black);
  tft.setRotation(1);
  tft.setCursor(0,0);
}

def screenReset() {
  // set background
  tft.fillScreen(black);

  // set initial graphic
  for (int r = 0; r < (width / 2); r += 25) {
    tft.drawCircle(width / 2, 0, r, green);
  }
}

def ledSetup() {
  ws2812fx.init();                     // 初始化LED
  ws2812fx.setBrightness(255);         // 设置亮度 (0-255)
  ws2812fx.setSpeed(200);              // 设置速度
  ws2812fx.setColor(0x007BFF);         // 设置颜色为天蓝色
  ws2812fx.setMode(FX_MODE_STATIC);    // 设置显示模式
  ws2812fx.start();                    // 启动显示
}

def ledReaction() {
  ws2812fx.service();                  // 维护LED显示更新

  // 每隔10秒改变效果
  static unsigned long last_change = 0;
  unsigned long now = millis();
  if(now - last_change > 10000) {
    int mode = (ws2812fx.getMode() + 1) % ws2812fx.getModeCount();
    ws2812fx.setMode(mode);            // 设置新的显示模式
    last_change = now;
  }
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

def sensorSetup(){
  for (int i = 0; i < numSensors; ++i) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
}

def getdistance(int sensor_num){
  // Clear the trigPin
  digitalWrite(trigPins[sensor_num], LOW);
  delayMicroseconds(2);
  // Send a 10-microsecond pulse to trigger the ultrasonic senso
  digitalWrite(trigPins[sensor_num], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPins[sensor_num], LOW);
  // Read the duration of the echo pulse
  duration = pulseIn(echoPin[sensor_num], HIGH);
  // Calculate the distance in centimeters (cm)
  distance = duration * 0.034 / 2;
  // Print the distance to the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000); // Wait for 1 second before taking the next measurement

}
