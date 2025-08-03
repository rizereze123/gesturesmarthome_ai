#define BLYNK_TEMPLATE_ID "TMPL6wHtu5yx1"
#define BLYNK_TEMPLATE_NAME "Smart Home AI"
#define BLYNK_AUTH_TOKEN "kRPn_V2dKSMeqL83W1Pb6dOn_L_ha5bG"

#include <ESP32Servo.h>
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

String inputString = "";
bool stringComplete = false;

#define relayKIPAS 33
#define LED 13
#define DHTPIN 26
#define DHTTYPE DHT11

DHT dht(DHTPIN,DHTTYPE);

float temp=0;
float hum=0;

BlynkTimer timer; // Creating a timer object

char ssid[] = "test";
char pass[] = "bismillah123";

void processCommand(String command);

Servo servo;

void setup()
{
  Serial.begin(115200);
  servo.setPeriodHertz(50); 
  servo.attach(12);

  pinMode(LED, OUTPUT);
  pinMode(relayKIPAS, OUTPUT);

  dht.begin();
  timer.setInterval(1000L, myTimerEvent); //Staring a timer
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);

  Serial.println("ESP32 Ready - Serial Communication");
  inputString.reserve(200);

  digitalWrite(LED, LOW);
  digitalWrite(relayKIPAS, LOW);
  servo.write(0);
  Blynk.virtualWrite(V2,"OFF");
  Blynk.virtualWrite(V3,"OFF");
  Blynk.virtualWrite(V4,"OFF");
}

void loop()
{
  // Check for serial data
  while (Serial.available())
  {
    char inChar = (char)Serial.read();

    if (inChar == '\n')
    {
      stringComplete = true;
    }
    else
    {
      inputString += inChar;
    }
  }

  // Process command when complete
  if (stringComplete)
  {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }

  temp=dht.readTemperature();
  hum=dht.readHumidity();
  Blynk.run();
  timer.run(); // runs the timer in the loop

}

void processCommand(String command)
{
  command.trim(); // Remove whitespace

  if (command == "index_on")
  {
    servo.write(90);
    Blynk.virtualWrite(V2,"ON");
  }
  else if (command == "index_off")
  {
    servo.write(0);
    Blynk.virtualWrite(V2,"OFF");
  }
  else if (command == "middle_on")
  {
    digitalWrite(relayKIPAS, HIGH);
    Blynk.virtualWrite(V3,"ON");
  }
  else if (command == "middle_off")
  {
    digitalWrite(relayKIPAS, LOW);
    Blynk.virtualWrite(V3,"OFF");
  }
  else if (command == "pinky_on")
  {
    digitalWrite(LED, HIGH);
    Blynk.virtualWrite(V4,"ON");
  }
  else if (command == "pinky_off")
  {
    digitalWrite(LED, LOW);
    Blynk.virtualWrite(V4,"OFF");
  }
  else if (command == "all_off")
  {
    digitalWrite(LED, LOW);
    digitalWrite(relayKIPAS, LOW);
    servo.write(0);
    Blynk.virtualWrite(V2,"OFF");
    Blynk.virtualWrite(V3,"OFF");
    Blynk.virtualWrite(V4,"OFF");
  }
  else
  {
    Serial.println("Unknown command: " + command);
  }
}

void myTimerEvent() // This loop defines what happens when timer is triggered
{

Blynk.virtualWrite(V0,temp);
Blynk.virtualWrite(V1, hum);

}