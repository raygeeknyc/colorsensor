/**
 * Use 3 CDS photosensors to read light color levels and PWM connected RGB LED to reflect the color.
 * Written for an Teensy 4.0.
 * 
 * 2019 by Raymond Blum <raygeeknyc@gmail.com>.
 */
// define _DEBUG for serial output
#define _NODEBUG

#define R_SENSOR_PIN A3
#define G_SENSOR_PIN A4
#define B_SENSOR_PIN A5

#define R_LED_PIN 12
#define G_LED_PIN 11
#define B_LED_PIN 10

#define TRAINING_MS 3000

#define CHANNEL_DELTA_THRESHOLD 100

#define LED_MIN 1
#define LED_MAX 250

#define SENSOR_MAX 1024
#define SENSOR_MIN 0

#define POLLING_DELAY_MS 100

unsigned int sensor_min, r_min, g_min, b_min;
unsigned int sensor_max, r_max, g_max, b_max;


// There's some language issues with using min and max functions - this is an easy workaround
inline int min_i(int a,int b) {return ((a)<(b)?(a):(b)); }
inline int max_i(int a,int b) {return ((a)>(b)?(a):(b)); }

void setSensorMax(const int reading) {
  sensor_max = max_i(sensor_max, reading);
}

void setSensorMin(const int reading) {
  sensor_min = min_i(sensor_min, reading);
}

// Recaord the highest sensor reading in a number of seconds to use as the initial scaling ceiling
void trainSensors() {  
  unsigned int r = analogRead(R_SENSOR_PIN);
  unsigned int g = analogRead(G_SENSOR_PIN);
  unsigned int b = analogRead(B_SENSOR_PIN);

  r_min = min_i(r, r_min);
  r_max = max_i(r, r_max);
  
  g_min = min_i(g, g_min);
  g_max = max_i(g, g_max);
  
  b_min = min_i(b, b_min);
  b_max = max_i(b, b_max);
  
  unsigned min_reading = min_i(r,min_i(g,b));
  unsigned max_reading = max_i(r,max_i(g,b));
    
  setSensorMin(r);
  setSensorMax(r);
  setSensorMin(g);
  setSensorMax(g);
  setSensorMin(b);
  setSensorMax(b);
}

void setup() {
  #ifdef  _DEBUG
  Serial.begin(115200);
  Serial.println("setup");
  #endif
  
  sensor_min = r_min = g_min = b_min = SENSOR_MAX;
  sensor_max = r_max = g_max = b_max = SENSOR_MIN;

  pinMode(R_SENSOR_PIN, INPUT_PULLDOWN);
  pinMode(G_SENSOR_PIN, INPUT_PULLDOWN);
  pinMode(B_SENSOR_PIN, INPUT_PULLDOWN);

  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);

  analogWrite(R_LED_PIN, 255);
  analogWrite(G_LED_PIN, 255);
  analogWrite(B_LED_PIN, 255);
  delay(400);
  analogWrite(R_LED_PIN, 0);
  analogWrite(G_LED_PIN, 0);
  analogWrite(B_LED_PIN, 0);
  delay(200);
  analogWrite(R_LED_PIN, 255);
  analogWrite(G_LED_PIN, 255);
  analogWrite(B_LED_PIN, 255);
  delay(400);
  analogWrite(R_LED_PIN, 0);
  analogWrite(G_LED_PIN, 0);
  analogWrite(B_LED_PIN, 0);

  #ifdef  _DEBUG
  Serial.println("training sensors");
  #endif

  unsigned long int start_at = millis();
  unsigned long int end_at = start_at + TRAINING_MS;
  while (millis() < end_at and millis() >= start_at) {
    trainSensors();
  }
  #ifdef  _DEBUG
  Serial.print("/trained sensors ");
  Serial.print("min: ");
  Serial.print(sensor_min);
  Serial.print(" max: ");
  Serial.println(sensor_max);
  #endif

  #ifdef _DEBUG
  Serial.println("/setup");
  #endif
}

bool is_max(const int test_i, const int control_1, const int control_2) {
  if (((test_i - control_1) > CHANNEL_DELTA_THRESHOLD) || ((test_i - control_2) > CHANNEL_DELTA_THRESHOLD))
    return true;
  else
    return false;
}

bool is_min(const int test_i, const int control_1, const int control_2) {
  if (((control_1 - test_i) > CHANNEL_DELTA_THRESHOLD) || ((control_2 - test_i) > CHANNEL_DELTA_THRESHOLD))
    return true;
  else
    return false;
}

unsigned int scale_input_to_range(const unsigned int input, const unsigned int ceiling) {
  float ratio_to_limit = (float)input/ceiling;
  return (unsigned int)(ratio_to_limit * sensor_max);
}

void showLeds() {
  unsigned int r_o, g_o, b_o;
  
  unsigned int r_raw = analogRead(R_SENSOR_PIN);
  unsigned int g_raw = analogRead(G_SENSOR_PIN);
  unsigned int b_raw = analogRead(B_SENSOR_PIN);

  unsigned int r_i = scale_input_to_range(r_raw, r_max);
  unsigned int g_i = scale_input_to_range(g_raw, g_max);
  unsigned int b_i = scale_input_to_range(b_raw, b_max);
  
  r_o = g_o = b_o = (LED_MAX + LED_MIN) / 5;

  if (is_max(r_i, g_i, b_i)) {
    r_o = LED_MAX;
  }
  if (is_max(g_i, r_i, b_i)) {
    g_o = LED_MAX;
  }
  if (is_max(b_i, r_i, g_i)) {
    b_o = LED_MAX;
  }

  if (is_min(r_i, g_i, b_i)) {
    r_o = LED_MIN;
  }
  if (is_min(g_i, r_i, b_i)) {
    g_o = LED_MIN;
  }
  if (is_min(b_i, r_i, g_i)) {
    b_o = LED_MIN;
  }

  #ifdef _DEBUG
  Serial.print(r_raw); 
  Serial.print(",");
  Serial.print(g_raw); 
  Serial.print(",");
  Serial.print(b_raw); 
  Serial.print(" : ");
  #endif
  
  #ifdef _DEBUG
  if (r_i > g_i && r_i > b_i) {
    Serial.print('*');
    Serial.print(r_i);
    Serial.print('*');
  } else {
    Serial.print(r_i);
  } 
  Serial.print(",");
  if (g_i > r_i && g_i > b_i) {
    Serial.print('*');
    Serial.print(g_i);
    Serial.print('*');
  } else {
    Serial.print(g_i);
  } 
  Serial.print(",");
  if (b_i > g_i && b_i > r_i) {
    Serial.print('*');
    Serial.print(b_i);
    Serial.print('*');
  } else {
    Serial.print(b_i);
  } 
  Serial.print(" : ");
  Serial.print(r_o);
  Serial.print(",");
  Serial.print(g_o);
  Serial.print(",");
  Serial.print(b_o);
  Serial.println();
  #endif
  
  analogWrite(R_LED_PIN, r_o);
  analogWrite(G_LED_PIN, g_o);
  analogWrite(B_LED_PIN, b_o);
}

void loop() {
  showLeds();
  delay(POLLING_DELAY_MS);
}
