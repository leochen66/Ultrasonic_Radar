
// Ultrasonic Sensor
// Screen stop moving: 21, 19, 5, 4
// Sensor gpio error: 36, 39
#define NUM_SENSORS 3

#define trigPin1 8
#define echoPin1 7
#define trigPin2 12
#define echoPin2 13
#define trigPin3 33
#define echoPin3 27


// LED Stripe
#define LED_COUNT 50
#define LED_PIN 14

#define TRANSITION_SPEED 400

enum LEDState {
  BLUE_BREATH,
  DISTANCE1,
  DISTANCE2,
  DISTANCE3,
};


// Screen
#define TFT_CS   15
#define TFT_DC   33
#define TFT_RST  -1

#define MAX_TRAILS 10

struct Trail {
  int mid_x;
  int mid_y;
  int end_x;
  int end_y;
};


// Screen States
#define NEAR_BOUND  50
#define FAR_BOUND   150

enum SensorState {
  SENSOR1_NEAR,
  SENSOR1_FAR,
  SENSOR1_NO,
  SENSOR2_NEAR,
  SENSOR2_FAR,
  SENSOR2_NO,
  SENSOR3_NEAR,
  SENSOR3_FAR,
  SENSOR3_NO
};
