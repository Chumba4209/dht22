#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// -------- WiFi --------
const char* ssid = "Lukrasta";
const char* password = "Cycy12345";

// -------- Flask server endpoint --------
String serverURL = "http://192.168.1.56:5000/api/dht22";

// -------- OLED settings --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------- DHT22 settings --------
#define DHTPIN 6        // Your actual data pin
#define DHTTYPE DHT22   // Choose DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // ----- Connect WiFi -----
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // ----- Initialize OLED -----
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();
  delay(1500);

  // ----- Initialize DHT22 -----
  dht.begin();
}

void loop() {
  // ----- Read DHT22 -----
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature(); // Celsius

  // Check for read errors
  if (isnan(humidity) || isnan(temp)) {
    Serial.println("DHT22 read error!");
  } 
  else {
    Serial.printf("Temp: %.2f°C  Humidity: %.2f%%\n", temp, humidity);
  }

  // ----- Update OLED -----
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("DHT22");

  display.setCursor(0, 20);
  display.printf("Temp: %.2f C", temp);

  display.setCursor(0, 40);
  display.printf("Humidity: %.2f %%", humidity);

  display.display();

  // ----- Send to MongoDB through Flask -----
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // JSON payload
    String jsonData = "{\"temperature\":" + String(temp) +
                      ",\"humidity\":" + String(humidity) + "}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("Server Response: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      Serial.println(http.getString());
    } else {
      Serial.println("Error sending POST request");
    }

    http.end();
  }

  delay(3000); // Read every 3 seconds
}



