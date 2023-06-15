#include "plantBot_ws2812.h"
#include "plantBot_sht20.h"
#include "OTA.h"
#include <Preferences.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <Wire.h>

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
int sda = 8;
int scl = 9;
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

  Wire.begin(8,9);
}

void loop()
{
  delay(10);
  ArduinoOTA.handle();

    byte error, address;
  int nDevices;
  TelnetStream.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      TelnetStream.print("I2C device found at address 0x");
      if (address<16)
         TelnetStream.print("0");
      TelnetStream.print(address,HEX);
      TelnetStream.println("  !");
      nDevices++;
     }
     else if (error==4)
     {
      TelnetStream.print("Unknown error at address 0x");
      if (address<16)
         TelnetStream.print("0");
      TelnetStream.println(address,HEX);
     }    
    }
    if (nDevices == 0)
       TelnetStream.println("No I2C devices found\n");
    else
       TelnetStream.println("done\n");
 }
