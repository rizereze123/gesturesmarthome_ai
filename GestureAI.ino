#define BLYNK_TEMPLATE_ID "TMPL6eG11jMrU"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN "a7f1XszW1I6M8kqzDyTjogJv4RhIS6OY"

#include <ESP32Servo.h>
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DFRobotDFPlayerMini.h>

// Pin Definition
#define RELAY_RING 32
#define LED_PINKY 13
#define LED_THUMB 27
#define DHTPIN 4
#define DHTTYPE DHT11
#define RAIN_SENSOR_PIN 34
#define RX_PIN 16
#define TX_PIN 17
#define LED_AC 21

DHT dht(DHTPIN, DHTTYPE);
Servo servo;
HardwareSerial mySerial(1);
DFRobotDFPlayerMini dfplayer;

char ssid[] = "esp32nih";
char pass[] = "12345678";

float temp = 0;
float hum = 0;

String inputString = "";
bool stringComplete = false;

BlynkTimer timer;

// Status per jari
bool indexActive  = false;
bool middleActive = false;
bool pinkyActive  = false;
bool thumbActive  = false;
bool ringActive   = false;

void myTimerEvent();
void processCommand(String command);
void rainCheck();
void checkConnection();

void setup() {
  Serial.begin(115200);

  // DFPlayer setup
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  if (!dfplayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini not detected!");
  } else {
    Serial.println("DFPlayer Mini online.");
    dfplayer.volume(20);
  }

  // Servo setup
  servo.setPeriodHertz(50);
  servo.attach(12);

  // Pin setup
  pinMode(LED_PINKY, OUTPUT);
  pinMode(RELAY_RING, OUTPUT);
  pinMode(LED_THUMB, OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(LED_AC, OUTPUT);

  // Default OFF
  digitalWrite(LED_PINKY, LOW);
  digitalWrite(RELAY_RING, LOW);
  digitalWrite(LED_THUMB, LOW);
  digitalWrite(LED_AC, LOW);
  servo.write(0);

  dht.begin();

  // Timer events
  timer.setInterval(1000L, myTimerEvent);
  timer.setInterval(2000L, rainCheck);
  timer.setInterval(10000L, checkConnection);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("ESP32 Ready - Serial Communication");
  inputString.reserve(200);
}

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }

  if (stringComplete) {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }

  Blynk.run();
  timer.run();
}

void myTimerEvent() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("°C  Humidity: ");
    Serial.print(hum);
    Serial.println("%");

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void rainCheck() {
  int rainValue = analogRead(RAIN_SENSOR_PIN);
  Serial.print("Rain sensor value: ");
  Serial.println(rainValue);

  if (rainValue < 2000) {
    servo.write(90);
    Blynk.virtualWrite(V2, "Closed by Rain");
  } else {
    servo.write(0);
    Blynk.virtualWrite(V2, "Open (Dry)");
  }
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.begin(ssid, pass);
  }
  if (!Blynk.connected()) {
    Serial.println("Blynk disconnected! Reconnecting...");
    Blynk.connect();
  }
}

void processCommand(String command) {
  command.trim();
  Serial.println("Command: " + command);

  // Update status jari
  if (command == "index_on")  indexActive = true;
  if (command == "index_off") indexActive = false;

  if (command == "middle_on")  middleActive = true;
  if (command == "middle_off") middleActive = false;

  if (command == "pinky_on")  pinkyActive = true;
  if (command == "pinky_off") pinkyActive = false;

  if (command == "thumb_on")  thumbActive = true;
  if (command == "thumb_off") thumbActive = false;

  if (command == "ring_on")  ringActive = true;
  if (command == "ring_off") ringActive = false;

  // === Logika ON/OFF berdasarkan kombinasi ===

  // 1. Peace sign (Index + Middle) → Servo
  if (indexActive && middleActive) {
    servo.write(90);
    Blynk.virtualWrite(V2, "ON");
  } else {
    servo.write(0);
    Blynk.virtualWrite(V2, "OFF");
  }

  // 2. Metal sign (Index + Pinky) → Relay + LED_AC
  if (indexActive && pinkyActive) {
    digitalWrite(RELAY_RING, HIGH);
    digitalWrite(LED_AC, HIGH);
    Blynk.virtualWrite(V3, "ON");
  } else {
    digitalWrite(RELAY_RING, LOW);
    digitalWrite(LED_AC, LOW);
    Blynk.virtualWrite(V3, "OFF");
  }

  // 3. Index + Thumb → LED_PINKY
  if (indexActive && thumbActive) {
    digitalWrite(LED_PINKY, HIGH);
    Blynk.virtualWrite(V4, "ON");
  } else {
    digitalWrite(LED_PINKY, LOW);
    Blynk.virtualWrite(V4, "OFF");
  }

  // 4. Ring finger → LED_THUMB
  if (ringActive) {
    digitalWrite(LED_THUMB, HIGH);
    Blynk.virtualWrite(V5, "ON");
  } else {
    digitalWrite(LED_THUMB, LOW);
    Blynk.virtualWrite(V5, "OFF");
  }

  // Debug status
  Serial.print("Index: "); Serial.print(indexActive);
  Serial.print(" | Middle: "); Serial.print(middleActive);
  Serial.print(" | Pinky: "); Serial.print(pinkyActive);
  Serial.print(" | Thumb: "); Serial.print(thumbActive);
  Serial.print(" | Ring: "); Serial.println(ringActive);
}
