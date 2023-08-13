#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "credentials.h"
// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "Plant-Bot"


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 79 bytes
  { 255,2,0,2,0,72,0,16,31,1,129,0,3,8,57,6,0,76,105,103,
  104,116,32,32,32,77,111,105,115,116,32,84,104,114,101,115,104,0,66,1,
  7,17,7,16,13,26,66,1,27,17,7,16,176,26,4,0,45,16,7,18,
  2,26,2,0,37,42,22,11,2,26,31,31,79,78,0,79,70,70,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t Threshold = 50; // =0..100 slider position 
  uint8_t PumpDirection; // =1 if switch ON and =0 if OFF 

    // output variables
  int8_t Light; // =0..100 level position 
  int8_t Moisture; // =0..100 level position 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)
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
bool messageSent = false;
bool timedOut = false;
int timeWatered = millis();
void update();
void setup() {
// attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  RemoteXY_Init();

  Serial.begin(115200);  // The baudrate of Serial monitor is set in 9600
  strip.begin();
  strip.setBrightness(10);

  pinMode(LIGHT_SIG, INPUT);
  pinMode(MOISTURE_SIG, INPUT);

  pinMode(IND_LED, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(LIGHT_PWR, OUTPUT);

  digitalWrite(LIGHT_PWR, HIGH);
}

void loop() {
  update();


  if(RemoteXY.Moisture < RemoteXY.Threshold && !timedOut)
  {
    digitalWrite(MOTOR_IN1, !RemoteXY.PumpDirection);
    digitalWrite(MOTOR_IN2, RemoteXY.PumpDirection);
    digitalWrite(IND_LED, HIGH);
    if(millis() > timeWatered + 60000)
    {
      timedOut = true;
    }
  }
  else
  {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(IND_LED, LOW);
    timeWatered = millis();
  }

  if(!messageSent && timedOut)
  {
    bot.sendMessage(CHAT_ID, "I'm out of water!", "");
    messageSent = true;
  }


}

void update() {

  RemoteXY_Handler();
  RemoteXY.Light = analogRead(LIGHT_SIG) / 40.96;
  delay(10);
  //  SOIL MOISTURE SENSOR (0-100%)
  RemoteXY.Moisture = 100 - 100.0 * (analogRead(MOISTURE_SIG) - 1800) / (2700 - 1800);  //mapping to max and min
  if (abs(RemoteXY.Moisture) > 1000 || RemoteXY.Moisture < 1) {
    RemoteXY.Moisture = 0;
  }
  Serial.print("Moist.: ");
  Serial.print(RemoteXY.Moisture);
  Serial.print(", Light: ");
  Serial.println(RemoteXY.Light);
  //  NEOPIXEL
  int col = int(RemoteXY.Moisture * 160 / 100);
  strip.setLedColorData(0, strip.Wheel((0 / 1 + col) & 255));
  strip.show();



}
