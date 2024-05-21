#include <SPI.h>
#include <WS2812FX.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

#define NUM_SENSORS 2

// Pins issue:
// Screen stop moving: 21, 19, 5, 4
// Sensor gpio error: 36, 39
#define trigPin1 8
#define echoPin1 7
#define trigPin2 21 //
#define echoPin2 19 //
#define trigPin3 5 //
#define echoPin3 4 //
#define trigPin4 12
#define echoPin4 13

#define LED_COUNT 50
#define LED_PIN 14

#define TFT_CS   15
#define TFT_DC   33
#define TFT_RST  -1

int width = 480;
int height = 320;
uint16_t black = 0x0000;
uint16_t green = 0x07E0;
uint16_t skyBlue = 0x007BFF;
uint16_t red = 0xF800;

// const int trigPins[NUM_SENSORS] = {trigPin1, trigPin2, trigPin3, trigPin4};
// const int echoPins[NUM_SENSORS] = {echoPin1, echoPin2, echoPin3, echoPin4};

const int trigPins[NUM_SENSORS] = {trigPin1, trigPin4};
const int echoPins[NUM_SENSORS] = {echoPin1, echoPin4};

// Screen controller
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// LED controller
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int LED_STATE = 0;
 
void setup() {
  Serial.begin(115200);
  delay(250);

  screenSetup();
  screenReset();
  delay(250);

  sensorSetup();
  delay(250);

  ledSetup();

  xTaskCreatePinnedToCore(
                    TaskSensor,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                    TaskLED,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500);
}

unsigned long previousLedMillis = 0;
const long ledInterval = 1000; // LED反应间隔为1秒

void loop() {
}

void TaskSensor(void * pvParameters){
  for (;;) {
    drawRadar();

    for (int i = 0; i < NUM_SENSORS; i++) {
      getdistance(i);
      vTaskDelay(10 / portTICK_PERIOD_MS); // Replace delay with vTaskDelay
    }
  }
}

void TaskLED(void * pvParameters){
  for (;;) {
    ledReaction();
  }
}

void screenSetup() {
  tft.begin();
  delay(1000);
  tft.fillScreen(black);
  tft.setRotation(1);
  tft.setCursor(0,0);
}

void screenReset() {
  // set background
  tft.fillScreen(black);

  // set initial graphic
  for (int r = 0; r < (width / 2); r += 25) {
    tft.drawCircle(width / 2, 0, r, green);
  }
}


void ledSetup() {
  ws2812fx.init();                     // 初始化LED
  ws2812fx.setBrightness(255);         // 设置亮度 (0-255)
  ws2812fx.setSpeed(2000);              // 设置速度
  ws2812fx.setColor(skyBlue);         // 设置颜色为天蓝色
  ws2812fx.setMode(FX_MODE_FADE);    // 设置显示模式
  ws2812fx.start();                    // 启动显示
}

void ledReaction() {
  ws2812fx.service();                  // 维护LED显示更新

  if (LED_STATE == 1) {
    ws2812fx.setColor(RED);            // Set color to red if LED_STATE is 1
  } else {
    ws2812fx.setColor(skyBlue);       // Set color to sky blue if LED_STATE is 0
  }
}

void drawRadar() {

  int prev_x = 0;
  int prev_y = 0;

  for (int angle = 0; angle < 190; angle += 10) {

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

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  for (int angle = 200; angle > -10; angle -= 10) {

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

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void sensorSetup(){
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
}

void getdistance(int sensor_num){
  long distances[NUM_SENSORS] = {0};

  for (int i = 0; i < NUM_SENSORS; i++) {
    long duration = 0;

    // Clear the trigPin
    digitalWrite(trigPins[sensor_num], LOW);
    delayMicroseconds(2);
    // Send a 10-microsecond pulse to trigger the ultrasonic senso
    digitalWrite(trigPins[sensor_num], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[sensor_num], LOW);
    // Read the duration of the echo pulse
    duration = pulseIn(echoPins[sensor_num], HIGH);
    
    // Calculate the distance in centimeters (cm)
    distances[i] = duration * 0.034 / 2;

    // Print the distance to the serial monitor
    Serial.print("Distance from sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println(" cm");

    vTaskDelay(10 / portTICK_PERIOD_MS); // Replace delay with vTaskDelay
  }

  bool anyLessThan50 = false;
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (distances[i] < 50) { // Adjust the distance threshold as needed
      anyLessThan50 = true;
      break; // Exit the loop early if any distance is less than 50
    }
  }

  if (anyLessThan50) {
    Serial.println("LED_STATE = 1");
    LED_STATE = 1;
  } else {
    Serial.println("LED_STATE = 0");
    LED_STATE = 0;
  }
  Serial.println();
}
