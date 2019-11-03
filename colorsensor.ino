/**
 * Use 3 CDS photosensors to read light color levels and PWM connected RGB LED to reflect the color.
 * Written for an Teensy 4.0.
 * 
 * 2019 by Raymond Blum <raygeeknyc@gmail.com>.
 */

#define R_SENSOR_PIN A3
#define G_SENSOR_PIN A4
#define B_SENSOR_PIN A5

#define R_LED_PIN 12
#define G_LED_PIN 11
#define B_LED_PIN 10

#define TRAINING_MS 3000

unsigned int sensor_min, sensor_max;
// There's some language issues with using min and max functions - this is an easy workaround
inline int min_i(int a,int b) {return ((a)<(b)?(a):(b)); }
inline int max_i(int a,int b) {return ((a)>(b)?(a):(b)); }

// Recaord the highest and lowest sensor readings in a number of seconds and take those as our range
void trainSensors() {
  unsigned long int start_at = millis();
  unsigned long int end_at = start_at + TRAINING_MS;

  sensor_min = 9999;
  sensor_max = 0;
  
  while (millis() < end_at and millis() >= start_at) {
    int t = analogRead(R_SENSOR_PIN);
    
    sensor_min = min_i(sensor_min, t);
    sensor_min = min_i(sensor_min, t);
    sensor_min = min_i(sensor_min, analogRead(B_SENSOR_PIN));
      
    sensor_max = max_i(sensor_max, analogRead(R_SENSOR_PIN));
    sensor_max = max_i(sensor_max, analogRead(G_SENSOR_PIN));
    sensor_max = max_i(sensor_max, analogRead(B_SENSOR_PIN));
  }
}

unsigned int scaleValue(const int raw_value, const int input_min, const int input_max) {
  #define OUTPUT_MAX 255
  
  return int(float(OUTPUT_MAX) * (float(raw_value - input_min) / (input_max - input_min)));
}

void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  pinMode(R_SENSOR_PIN, INPUT);
  pinMode(G_SENSOR_PIN, INPUT);
  pinMode(B_SENSOR_PIN, INPUT);

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

  Serial.println("training sensors");
  trainSensors();
  Serial.print("/trained sensors ");
  Serial.print("min: ");
  Serial.print(sensor_min);
  Serial.print(" max: ");
  Serial.println(sensor_max);
  
  Serial.println("/setup");
}

void showLeds() {
  unsigned int r_i = analogRead(R_SENSOR_PIN);
  unsigned int g_i = analogRead(G_SENSOR_PIN);
  unsigned int b_i = analogRead(B_SENSOR_PIN);

  unsigned int r_o = scaleValue(r_i, sensor_min, sensor_max);
  unsigned int g_o = scaleValue(g_i, sensor_min, sensor_max);
  unsigned int b_o = scaleValue(b_i, sensor_min, sensor_max);

  Serial.print(r_i);
  Serial.print(",");
  Serial.print(g_i);
  Serial.print(",");
  Serial.print(b_i);
  Serial.print(" : ");
  Serial.print(r_o);
  Serial.print(",");
  Serial.print(g_o);
  Serial.print(",");
  Serial.print(b_o);
  Serial.println();

  analogWrite(R_LED_PIN, r_o);
  analogWrite(G_LED_PIN, g_o);
  analogWrite(B_LED_PIN, b_o);
}
void loop() {
  showLeds();
  delay(100);
}
