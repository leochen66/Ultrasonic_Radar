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

unsigned long lastChange = 0;
int state = 0;
bool ledNeedReset = false;

const int pattern1[] = {0, 1, 2, 3, 4, 8, 11, 16, 19, 25, 27, 32, 35, 39, 41, 42, 43, 44};
const int pattern2[] = {5, 6, 7, 12, 15, 20, 24, 28, 31, 36, 37, 38};
const int pattern3[] = {13, 14, 21, 23, 29, 30};
const int pattern4[] = {9, 10, 17, 18, 26, 33, 34, 40, 45};


// Screen controller
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// LED controller
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int led_state = BLUE_BREATH;
 
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
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(2000);
  ws2812fx.setColor(skyBlue);
  ws2812fx.setMode(FX_MODE_FADE);
  ws2812fx.start();
}

void keepPattern4Off() {
  for (int i = 0; i < sizeof(pattern4) / sizeof(pattern4[0]); i++) {
    ws2812fx.setPixelColor(pattern4[i], BLACK);
  }
}

uint16_t CustomMode1(void) {

  for (int i = 0; i < sizeof(pattern2); i++) {
    ws2812fx.setPixelColor(pattern2[i], skyBlue);
  }
  for (int i = 0; i < sizeof(pattern1); i++) {
    ws2812fx.setPixelColor(pattern1[i], RED);
  }

  ws2812fx.setPixelColor(7, skyBlue);
  ws2812fx.setPixelColor(12, skyBlue);
  ws2812fx.setPixelColor(13, skyBlue);
  ws2812fx.setBrightness(255);

  keepPattern4Off();
  ws2812fx.show();
  return 0;
}

uint16_t CustomMode2(void) {

  
  for (int i = 0; i < sizeof(pattern3); i++) {
    ws2812fx.setPixelColor(pattern3[i], skyBlue);
  }
  for (int i = 0; i < sizeof(pattern1); i++) {
    ws2812fx.setPixelColor(pattern1[i], RED);
  }
  for (int i = 0; i < sizeof(pattern2); i++) {
    ws2812fx.setPixelColor(pattern2[i], RED);
  }

  ws2812fx.setPixelColor(13, skyBlue);
  ws2812fx.setPixelColor(7, RED);
  ws2812fx.setPixelColor(12, RED);
  ws2812fx.setBrightness(255);

  keepPattern4Off();
  ws2812fx.show();
  return 0;
}

uint16_t CustomMode3(void) {

  for (int i = 0; i < sizeof(pattern1); i++) {
    ws2812fx.setPixelColor(pattern1[i], RED);
  }
  for (int i = 0; i < sizeof(pattern2); i++) {
    ws2812fx.setPixelColor(pattern2[i], RED);
  }
  for (int i = 0; i < sizeof(pattern3); i++) {
    ws2812fx.setPixelColor(pattern3[i], RED);
  }
  ws2812fx.setBrightness(255);

  keepPattern4Off();
  ws2812fx.show();
  return 0;
}

void ledReaction() {
  if (led_state == BLUE_BREATH && ledNeedReset) {
    ledNeedReset = false;
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(2000);
    ws2812fx.setColor(skyBlue);
    ws2812fx.setMode(FX_MODE_FADE);
  }
  else if (led_state == DISTANCE1) {
    ws2812fx.setCustomMode(CustomMode1);
    ws2812fx.setMode(FX_MODE_CUSTOM);
  }
  else if (led_state == DISTANCE2) {
    ws2812fx.setCustomMode(CustomMode2);
    ws2812fx.setMode(FX_MODE_CUSTOM);
  }
  else if (led_state == DISTANCE3) {
    ws2812fx.setCustomMode(CustomMode3);
    ws2812fx.setMode(FX_MODE_CUSTOM);
  }

  ws2812fx.service();
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

  // find min distance
  long minDistance = distances[0];
  for (int i = 1; i < NUM_SENSORS; i++) {
      if (distances[i] < minDistance) {
          minDistance = distances[i];
      }
  }

  if (minDistance < 50){
    Serial.println("Set Sate: DISTANCE3");
    led_state = DISTANCE3;
  } 
  else if (minDistance >= 50 && minDistance < 100){
    Serial.println("Set Sate: DISTANCE2");
    led_state = DISTANCE2;
  }
  else if (minDistance >= 100 && minDistance < 150){
    Serial.println("Set Sate: DISTANCE1");
    led_state = DISTANCE1;
  }
  else{
    led_state = BLUE_BREATH;
    ledNeedReset = true;
  }

  Serial.println();
}