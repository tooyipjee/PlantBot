#include "plantBot_ws2812.h"
#include "plantBot_sht20.h"
#include "OTA.h"
#include <Preferences.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1       /* Time ESP32 will go to sleep (in hrs) */ 
#define WIFI_PIN  9

double timeToSLeep = uS_TO_S_FACTOR*TIME_TO_SLEEP;
//  PIN ASSIGNMENTS
int sig_read = 4;
int sht_pwr = 3;
int light_mic_pwr = 2;
int mic_sig = 1;
int light_sig = 0;

int in1 = 6;
int in2 = 7;
int neo_pin = 10;
int blk = 5;

//  VARIABLES
int col = 0;
int cycle_time = 2000; //ms
int last_switch_ts = 0;
bool pumpOn = false;
int targetMoisture = 12;
int hysteresis = 5;
bool timeOut = false;
int lastDataSendTs = 0;
int durationPumpOn = 0;

//  PERIPHERALS
plantBot_ws2812 strip = plantBot_ws2812(1, neo_pin, 0, TYPE_GRB);
plantBot_sht20 sht20(&Wire, SHT20_I2C_ADDR);
WiFiManager wm;
WiFiManagerParameter custom_param_timezone("tz", "Timezone", "", 40);
WiFiManagerParameter custom_param_weather("wt", "Location", "", 40);

String timezone;
String weather;
Preferences preferences;
void setup()
{
  Serial.begin(115200); // The baudrate of Serial monitor is set in 9600
  preferences.begin("credentials", false); 
  wm.addParameter(&custom_param_timezone);
  wm.addParameter(&custom_param_weather);
  wm.setSaveParamsCallback(saveParamsCallback);
  delay(1000);
  // Configure and start the WiFi station
  WiFi.mode(WIFI_STA);
//  Reset the WM settings to relaunch web portal.
  if (digitalRead(WIFI_PIN) == LOW)
  {
    wm.resetSettings();
  }
  bool res;
  res = wm.autoConnect("PlantBot_AP", "elektroThing"); // password protected app
  if (!res) {
    Serial.println("Failed to connect");
    wm.setConfigPortalTimeout(60);
    ESP.restart();
  }
  setupOTAWiFiMuted("PlantBot");

  pinMode(light_sig, INPUT);
  pinMode(mic_sig, INPUT);
  pinMode(sig_read, INPUT);

  pinMode(blk, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(light_mic_pwr, OUTPUT);
  pinMode(sht_pwr, OUTPUT);
  digitalWrite(sht_pwr, HIGH);
  digitalWrite(light_mic_pwr, HIGH);

  strip.begin();
  strip.setBrightness(10);

  sht20.initSHT20();
  delay(100);
  Serial.println("Sensor init finish!");
  sht20.checkSHT20();
  
}

void loop()
{
  delay(10);
  ArduinoOTA.handle();
  //  SHT20
  float humd = sht20.readHumidity(); //Read the measured data of air humidity
  float temp = sht20.readTemperature(); //Read the measured temp data
//
  //  SOIL MOISTURE SENSOR (0-100%)
  float moisture = 100 - 100.0 * (analogRead(sig_read) - 1800) / (2700 - 1800); //mapping to max and min
  if (abs(moisture) > 1000 || moisture < 1)
  {
    moisture = 0;
  }
  TelnetStream.print("Moist.: "); TelnetStream.print(moisture); 
  TelnetStream.print(", Temp.: "); TelnetStream.print(temp); 
  TelnetStream.print(", Humd.: "); TelnetStream.print(humd); 
  TelnetStream.print(", Mic.: "); TelnetStream.print(analogRead(mic_sig)); 
  TelnetStream.print(", Light: "); TelnetStream.print(analogRead(light_sig)); 
  TelnetStream.print(", Pump: "); TelnetStream.println(pumpOn); 
  //  NEOPIXEL
  col = int(moisture * 160 / 100);
  strip.setLedColorData(0, strip.Wheel((0 / 1 + col) & 255));
  strip.show();

 //  WATER PUMP
 if (millis() - last_switch_ts > 30000 && pumpOn == true) //if time
 {
   TelnetStream.println("ERROR MOTOR TIMED OUT");
   timeOut = true;
   //    Motor OFF
   digitalWrite(in1, LOW);
   digitalWrite(in2, LOW);

   digitalWrite(blk, HIGH);
   delay(500);
   digitalWrite(blk, LOW);
   delay(500);
   digitalWrite(blk, HIGH);
 }
 else
 {
   if (moisture < (targetMoisture - hysteresis))
   { 
     //    Motor ON
     digitalWrite(in1, HIGH);
     digitalWrite(in2, LOW);
     pumpOn = true;

   }
   else if (moisture > (targetMoisture + hysteresis))
   {
     //    Motor OFF
     digitalWrite(in1, LOW);
     digitalWrite(in2, LOW);
     pumpOn = false;
     last_switch_ts = millis();

   }
 }
}

void saveParamsCallback () {
  String tz = custom_param_timezone.getValue();
  String wt = custom_param_weather.getValue();


  Serial.print("Timezone: "); Serial.println(tz);
  Serial.print("Weather: "); Serial.println(wt);

  preferences.putString("timezone", tz); 
  preferences.putString("weather", wt); 

}

