#define BLYNK_TEMPLATE_ID "TMPL31L-uJNL9"
#define BLYNK_TEMPLATE_NAME "SMART AIR MONITORING SYSTEM"
#define BLYNK_AUTH_TOKEN "Your_Auth_Token"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MQ135_PIN 34
#define DHTPIN 4
#define DHTTYPE DHT11
#define LED_PIN 15
#define RELAY_PIN 5

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHT dht(DHTPIN, DHTTYPE);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "SSID_NAME";
char pass[] = "PASSWORD";

float mqToPPM(int raw) {
  return map(raw, 0, 4095, 50, 600);
}

void setup() {

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Air Quality Monitor");
  display.display();

  Blynk.begin(auth, ssid, pass);

  delay(1500);
}

void loop() {
  Blynk.run();

  int raw = analogRead(MQ135_PIN);
  float ppm = mqToPPM(raw);

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t)) t = 0;
  if (isnan(h)) h = 0;

  Blynk.virtualWrite(V0, ppm);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);

  String status = "";

  if (ppm < 100) {
    status = "GOOD";
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
  else if (ppm >= 100 && ppm < 200) {
    status = "MODERATE";
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
  else if (ppm >= 200 && ppm < 300) {
    status = "POOR";
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Blynk.logEvent("air_alert", "Air quality getting poor!");
  }
  else {
    status = "HAZARDOUS";
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Blynk.logEvent("air_alert", "Hazardous air quality!");
  }

  Blynk.virtualWrite(V3, status);

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Air Quality: ");
  display.print(ppm);
  display.println(" PPM");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(t);
  display.println(" C");

  display.setCursor(0, 28);
  display.print("Humidity: ");
  display.print(h);
  display.println(" %");

  display.setCursor(0, 44);
  display.print("Status: ");
  display.print(status);

  display.display();

  delay(2000);
}