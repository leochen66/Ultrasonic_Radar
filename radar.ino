#include <SPI.h>
#include <WS2812FX.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include "myhead.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

// Define the current state for each sensor
SensorState sensor1State = SENSOR1_FAR;
SensorState sensor2State = SENSOR2_FAR;
SensorState sensor3State = SENSOR3_NO;

int width = 480;
int height = 320;
uint16_t black = 0x0000;
uint16_t green = 0x07E0;
uint16_t skyBlue = 0x007BFF;
uint16_t red = 0xF800;

Trail trails[MAX_TRAILS];
int trailIndex = 0;

const int trigPins[NUM_SENSORS] = {trigPin1, trigPin2, trigPin3};
const int echoPins[NUM_SENSORS] = {echoPin1, echoPin2, echoPin3};

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
  delay(250);

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

void loop() {
}

void TaskSensor(void * pvParameters){
  for (;;) {
    drawRadar(false);
    runSensor();
    drawRadar(true);
    runSensor();
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

void drawRadar(bool reverse) {
  int prev_x = 0;
  int prev_y = 0;

  int start_angle = reverse ? 0 : 200;
  int end_angle = reverse ? 200 : -20;
  int step = reverse ? 2 : -2;

  for (int i = start_angle; reverse ? (i < end_angle) : (i > end_angle); i += step) {
    int angle = i;
    
    // Clear the oldest trail
    if (trailIndex >= MAX_TRAILS) {
      int clearIndex = (trailIndex + 1) % MAX_TRAILS;
      tft.drawLine(width / 2, 0, trails[clearIndex].end_x, trails[clearIndex].end_y, black);
      tft.drawLine(width / 2, 0, trails[clearIndex].mid_x, trails[clearIndex].mid_y, black);
      tft.drawLine(trails[clearIndex].mid_x, trails[clearIndex].mid_y, trails[clearIndex].end_x, trails[clearIndex].end_y, black);
    }

    // Calculate the radius of the outermost concentric circle
    int max_radius = (width / 2) - 25;

    // Calculate the end point of the line
    int end_x = width / 2 + cos(PI / 180 * angle) * max_radius;
    int end_y = sin(PI / 180 * angle) * max_radius;
    int mid_x = width / 2 + cos(PI / 180 * angle) * (max_radius / 2);
    int mid_y = sin(PI / 180 * angle) * (max_radius / 2);

    // Determine the current sensor state based on the angle
    SensorState currentState;
    if (angle >= 0 && angle < 60) {
      currentState = sensor1State;
    } else if (angle >= 60 && angle < 120) {
      currentState = sensor2State;
    } else if (angle >= 120 && angle < 180) {
      currentState = sensor3State;
    } else {
      currentState = SENSOR1_NO; // Default to SENSOR1_NO for angles outside 0-180
    }

    // Draw the line based on the current sensor state
    switch (currentState) {
      case SENSOR1_NEAR:
      case SENSOR2_NEAR:
      case SENSOR3_NEAR:
        // Draw the entire line in red
        tft.drawLine(width / 2, 0, end_x, end_y, red);
        // Update the trail to full length
        trails[trailIndex % MAX_TRAILS].mid_x = end_x;
        trails[trailIndex % MAX_TRAILS].mid_y = end_y;
        trails[trailIndex % MAX_TRAILS].end_x = end_x;
        trails[trailIndex % MAX_TRAILS].end_y = end_y;
        break;
      case SENSOR1_FAR:
      case SENSOR2_FAR:
      case SENSOR3_FAR:
        // Draw half the line in red and half in green
        tft.drawLine(width / 2, 0, mid_x, mid_y, red);
        tft.drawLine(mid_x, mid_y, end_x, end_y, green);
        // Update the trail to the midpoint and full length
        trails[trailIndex % MAX_TRAILS].mid_x = mid_x;
        trails[trailIndex % MAX_TRAILS].mid_y = mid_y;
        trails[trailIndex % MAX_TRAILS].end_x = end_x;
        trails[trailIndex % MAX_TRAILS].end_y = end_y;
        break;
        break;
      case SENSOR1_NO:
      case SENSOR2_NO:
      case SENSOR3_NO:
      default:
        // Draw the entire line in green
        tft.drawLine(width / 2, 0, end_x, end_y, green);
        // Update the trail to full length
        trails[trailIndex % MAX_TRAILS].mid_x = end_x;
        trails[trailIndex % MAX_TRAILS].mid_y = end_y;
        trails[trailIndex % MAX_TRAILS].end_x = end_x;
        trails[trailIndex % MAX_TRAILS].end_y = end_y;
        break;
    }

    // Update the trail
    trailIndex++;

    vTaskDelay(13 / portTICK_PERIOD_MS);
  }
}

void sensorSetup(){
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
}

void runSensor(){
  long distances[NUM_SENSORS] = {0};

  for (int i = 0; i < NUM_SENSORS; i++) {
    long duration = 0;

    // Clear the trigPin
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(2);
    // Send a 10-microsecond pulse to trigger the ultrasonic senso
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);
    // Read the duration of the echo pulse
    duration = pulseIn(echoPins[i], HIGH);
    
    // Calculate the distance in centimeters (cm)
    distances[i] = duration * 0.034 / 2;

    // Print the distance to the serial monitor
    Serial.print("Distance from sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println(" cm");

    // Update Screen States
    if (i == 0 && distances[i] > FAR_BOUND)
      sensor1State = SENSOR1_NO;
    else if (i == 1 && distances[i] > FAR_BOUND)
      sensor2State = SENSOR2_NO;
    else if (i == 2 && distances[i] > FAR_BOUND)
      sensor3State = SENSOR3_NO;
    else if (i == 0 && distances[i] < NEAR_BOUND)
      sensor1State = SENSOR1_NEAR;
    else if (i == 1 && distances[i] < NEAR_BOUND)
      sensor2State = SENSOR2_NEAR;
    else if (i == 2 && distances[i] < NEAR_BOUND)
      sensor3State = SENSOR3_NEAR;
    else if (i == 0 && distances[i] < FAR_BOUND && distances[i] > NEAR_BOUND)
      sensor1State = SENSOR1_FAR;
    else if (i == 1 && distances[i] < FAR_BOUND && distances[i] > NEAR_BOUND)
      sensor2State = SENSOR2_FAR;
    else if (i == 2 && distances[i] < FAR_BOUND && distances[i] > NEAR_BOUND)
      sensor3State = SENSOR3_FAR;

    vTaskDelay(100 / portTICK_PERIOD_MS);
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



// Note
// Sepearate screen task
// change mode logic
