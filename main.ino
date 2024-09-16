#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

Adafruit_MPU6050 mpu;
float past;
bool is_read = 1;
const int ledPin = D0;
const int buzzerPin = D3;
const int buttonPin = D4; 
bool buttonPressed = false;
const char *ssid = "mora";
const char *password = "mora1234";
const String apiKey = "G9AQJ7HVZXM5OWFN";
const String server = "api.thingspeak.com";

void buttonInterrupt();
void sendToThingSpeak(int field, int value);

void setup(void) {
  Serial.begin(115200);
  while (!Serial)delay(10);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {delay(10);}
  }
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, CHANGE);
}
bool is_falling = false;
void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  sendToThingSpeak(1, a.acceleration.z);
Serial.print("acceleration: ");
Serial.println(a.acceleration.z);
  if(is_read){
    past = a.acceleration.z;
    is_read = 0;
  } else {
    float current = a.acceleration.z;
    float delta = current-past;
    sendToThingSpeak(2, delta);
    if(abs(delta) >= 2.0)is_falling = true;
    if(is_falling){
       digitalWrite(ledPin, HIGH);
       delay(4000);
       if(buttonPressed){
          digitalWrite(ledPin, LOW);
          digitalWrite(buzzerPin,LOW);
          is_falling = false; 
          buttonPressed = false;
       } else {
          digitalWrite(buzzerPin,HIGH);
          delay(4000);
       } 
    }
    past = current;
  }
  delay(500);
}

void ICACHE_RAM_ATTR buttonInterrupt() {
  buttonPressed = true;
}

void sendToThingSpeak(int field, int value) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = "http://" + server + "/update?api_key=" + apiKey + "&field" + String(field)+ "=" + String(value);
    Serial.println("Sending to ThingSpeak: " + url);

    http.begin(client, url);
    int httpCode = http.GET();

    Serial.println("HTTP response code: " + String(httpCode));
    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send data to ThingSpeak.");
  }
}