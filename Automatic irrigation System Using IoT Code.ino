#include <Wire.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#define DHT11PIN 16

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHT11PIN, DHT11);

const int soilPin = 39;
#define rainPin 36
#define lowPin 34
#define highPin 35
#define motor  4

#define RXD2 1  //(RX2)
#define TXD2 3 //(TX2)
#define gsm Serial2

int soilValue = 0;
int rainValue = 0;
int lowValue = 0;
int highValue = 0;
String waterStatus;

const char* ssid = "project";   // your network SSID (name)
const char* password = "123456789"; // your network password

WiFiClient  client;

unsigned long myChannelNumber = 1535996;
const char * myWriteAPIKey = "3KVUJJOWQH2FJQ79";

void setup() {
  Serial.begin(115200);
  delay(500);

  dht.begin();
  delay(500);

  gsm.begin(9600);
  delay(500);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("AUTOMATIC IRRIGATION SYSTEM WITH WATER-LEVEL INDICATOR");
  display.display();
  Serial.println("AUTOMATIC IRRIGATION SYSTEM WITH WATER-LEVEL INDICATOR");

  pinMode(motor, OUTPUT);
  pinMode(soilPin, INPUT);
  pinMode(rainPin, INPUT);
  pinMode(lowPin, INPUT);
  pinMode(highPin, INPUT);
  delay(500);

  digitalWrite(motor, LOW);
  delay(500);

  gsm.println("AT");
  delay(1500);
  gsm.println("AT+CREG?");
  delay(1500);
  gsm.println("AT+CMGF=1");
  delay(1500);
  gsm.println("AT+CNMI=1,2,0,0");
  delay(1500);

  display.setCursor(0, 20);            // Start at top-left corner
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE, BLACK);       // Draw white text
  display.println(("Attempt to connect"));
  display.display();
  delay(1000);

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect");
    delay(1000);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      delay(5000);
    }
  }
  display.setCursor(0, 30);            // Start at top-left corner
  display.println(("WiFi connected..!"));
  Serial.println("WiFi connected..!");
  display.display();
  delay(1000);

  display.setCursor(0, 40);            // Start at top-left corner
  display.setTextColor(WHITE, BLACK);
  display.println(("Got IP: "));
  display.println(WiFi.localIP());
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
  display.display();
  delay(2000);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop()
{
  display.clearDisplay();
  delay(500);
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Reading Sensors ..."));
  display.display();
  delay(500);

  float temp = dht.readTemperature();
  display.setCursor(0, 20);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Temperature = "));
  display.print(temp);
  display.print("C");
  display.display();
  ThingSpeak.setField(1, temp);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(500);
  if (temp > 40)
  {
    display.setCursor(0, 30);
    display.setTextColor(WHITE, BLACK);
    display.print(F("Temperature Abnormal"));
    display.display();
    sendSMS("Temperature Abnormal");
    delay(2000);
  }
  else
  {
    digitalWrite(motor, LOW);
    delay(2000);
  }

  float humi = dht.readHumidity();
  display.setCursor(0, 40);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Humidity = "));
  display.print(humi);
  display.display();
  ThingSpeak.setField(2, humi);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(500);
  if (humi > 60)
  {
    display.setCursor(0, 50);
    display.setTextColor(WHITE, BLACK);
    display.print(F("Humidity Abnormal"));
    display.display();
    sendSMS("Humidity Abnormal");
    delay(2000);
  }
  else
  {
    digitalWrite(motor, LOW);
    delay(2000);
  }

  display.clearDisplay();
  delay(500);
  soilValue = analogRead(soilPin);
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Soil Moisture = "));
  display.print(soilValue);
  display.display();
  ThingSpeak.setField(3, soilValue);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(500);
  if (soilValue > 870)
  {
    digitalWrite(motor, HIGH);
    display.setCursor(0, 10);
    display.setTextColor(WHITE, BLACK);
    display.print(F("DRY Soil ..."));
    display.display();
    sendSMS("DRY Soil ...");
    delay(2000);
  }
  else
  {
    display.setCursor(0, 10);
    display.setTextColor(WHITE, BLACK);
    display.print(F("Moisture Soil ..."));
    display.display();
    digitalWrite(motor, LOW);
    delay(2000);
  }

  rainValue = analogRead(rainPin);
  display.setCursor(0, 20);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Rain Level = "));
  display.print(rainValue);
  display.display();
  ThingSpeak.setField(4, rainValue);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(500);
  if (rainValue < 870)
  {
    display.setCursor(0, 30);
    display.setTextColor(WHITE, BLACK);
    display.print(F("Rain Falling ..."));
    display.display();
    sendSMS("Rain Falling ...");
    delay(2000);
  }
  else
  {
    digitalWrite(motor, LOW);
    delay(2000);
  }

  display.setCursor(0, 40);
  display.setTextColor(WHITE, BLACK);
  display.print(F("Water Status: "));
  display.print(digitalRead(lowPin));
  display.print(F("   "));
  display.print(digitalRead(highPin));
  display.display();
  delay(200);

  if (digitalRead(lowPin) == HIGH)
  {
    waterStatus = "Low-Level Water";
    digitalWrite(motor, HIGH);
    delay(2000);
    if (digitalRead(highPin) == HIGH)
    {
      waterStatus = "High-Level Water";
      digitalWrite(motor, LOW);
      sendSMS("High-Level Water");
      delay(2000);
    }
  }
  display.setCursor(0, 50);
  display.setTextColor(WHITE, BLACK);
  display.print(waterStatus);
  display.display();
  delay(200);
}


void sendSMS(String message)
{
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print("Sending SMS...");
  display.display();
  delay(200);
  Serial.print("Sending SMS...");
  gsm.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(200);
  gsm.println("AT+CMGS=\"mobile no\"\r\n");  // recipient's mobile number, in international format
  delay(1000);
  gsm.println(message);                         // message to send
  gsm.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(200);                                     // give module time to send SMS
}
