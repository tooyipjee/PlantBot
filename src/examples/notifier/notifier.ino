#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "credentials.h"


#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;        /* Time ESP32 will go to sleep (in seconds) */
#define LIGHT_SIG 0
#define LIGHT_PWR 2
#define MOISTURE_SIG 4
#define IND_LED 5
#define MOTOR_IN1 6
#define MOTOR_IN2 7
#define WS2812 10

#define LEDS_COUNT 1
//  PERIPHERALS
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, WS2812, 0);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
WiFiManager wm;


bool messageSent = false;
int lastGoodTS = millis();
bool timedOut = false;
int Light = 0;
int Moisture = 0;
void update();
void setup() {
  Serial.begin(115200);  // The baudrate of Serial monitor is set in 9600

  pinMode(LIGHT_SIG, INPUT);
  pinMode(MOISTURE_SIG, INPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(IND_LED, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(LIGHT_PWR, OUTPUT);
  digitalWrite(LIGHT_PWR, HIGH);



  WiFi.mode(WIFI_STA);
  bool res;
  res = wm.autoConnect("PlantBot_AP", "elektroThing");  // password protected app
  if (!res) {
    Serial.println("Failed to connect");
    wm.setConfigPortalTimeout(60);
    ESP.restart();
  }
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  update();
  bot.sendMessage(CHAT_ID, "Light: " + String(Light) + ", Moist:" + String(Moisture), "");


  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");

}

void loop() {
}

void update() {
  digitalWrite(IND_LED, HIGH);

  delay(1000);

  Light = analogRead(LIGHT_SIG) / 40.96;
  //  SOIL MOISTURE SENSOR (0-100%)
  Moisture = 100 - 100.0 * (analogRead(MOISTURE_SIG) - 1800) / (2700 - 1800);  //mapping to max and min
  if (abs(Moisture) > 100 || Moisture < 1) {
    Moisture = 0;
  }
  Serial.print("Moist.: ");
  Serial.print(Moisture);
  Serial.print(", Light: ");
  Serial.println(Light);
  digitalWrite(LIGHT_PWR, LOW);
  digitalWrite(IND_LED, LOW);
}
