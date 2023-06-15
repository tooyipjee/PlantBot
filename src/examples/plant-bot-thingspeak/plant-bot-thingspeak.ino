#include "plantBot_ws2812.h"
#include "plantBot_sht20.h"
#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "VM5191158";   // your network SSID (name)
const char* password = "kgLpwnrX3jrr";   // your network password
WiFiClient  client;
unsigned long myChannelNumber = 1703519;
const char * myWriteAPIKey = "THDGFM62ZIH3Y8F7";
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1       /* Time ESP32 will go to sleep (in hrs) */ 
double timeToSLeep = uS_TO_S_FACTOR*TIME_TO_SLEEP;
//  PIN ASSIGNMENTS
int led_sig = 3;
int sig_read = 4;
int sht_pwr = 7;
int light_mic_pwr = 2;
int mic_sig = 3;
int light_sig = 6;
int sda = 19;
int scl = 18;
int in1 = 0;
int in2 = 1;
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

void setup()
{
  Serial.begin(115200); // The baudrate of Serial monitor is set in 9600
  while (!Serial); // Waiting for Serial Monitor
  pinMode(sig_read, INPUT);
  pinMode(blk, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(light_mic_pwr, OUTPUT);
  pinMode(sht_pwr, OUTPUT);
  digitalWrite(sht_pwr, HIGH);
  strip.begin();
  strip.setBrightness(100);

  sht20.initSHT20();
  delay(100);
  Serial.println("Sensor init finish!");
  sht20.checkSHT20();

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop()
{
//  digitalWrite(sht_pwr, HIGH);
//  digitalWrite(light_mic_pwr, HIGH);
//  Serial.print("Light: ");
//  Serial.print(analogRead(light_sig));
//  Serial.print(", Mic: ");
//  Serial.print(analogRead(mic_sig));
//  delay(500);
//  
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
  
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
//  
//  //  NEOPIXEL
//  col = int(moisture * 160 / 100);
//  strip.setLedColorData(0, strip.Wheel((0 / 1 + col) & 255));
//  strip.show();
//
//  Serial.println(millis() - last_switch_ts);
//  Serial.println(pumpOn);
//
//  //  WATER PUMP
//  if (millis() - last_switch_ts > 30000 && pumpOn == true) //if time
//  {
//    Serial.println("ERROR MOTOR TIMED OUT");
//    timeOut = true;
//    //    Motor OFF
//    digitalWrite(in1, LOW);
//    digitalWrite(in2, LOW);
//
//    digitalWrite(blk, HIGH);
//    delay(500);
//    digitalWrite(blk, LOW);
//    delay(500);
//    digitalWrite(blk, HIGH);
//  }
//  else
//  {
//    if (moisture < (targetMoisture - hysteresis))
//    { 
//      //    Motor ON
//      digitalWrite(in1, HIGH);
//      digitalWrite(in2, LOW);
//      pumpOn = true;
//
//    }
//    else if (moisture > (targetMoisture + hysteresis))
//    {
//      //    Motor OFF
//      digitalWrite(in1, LOW);
//      digitalWrite(in2, LOW);
//      pumpOn = false;
//      last_switch_ts = millis();
//
//    }
//  }
//
//  digitalWrite(blk, pumpOn);
//  digitalWrite(led_sig, LOW);
//
//  //  PRINT OUTPUT
//  Serial.print(" Temperature:");
//  Serial.print(temp, 1);   // Only print one decimal place
//  Serial.print("C");
//  Serial.print(" Humidity:");
//  Serial.print(humd, 1);   // Only print one decimal place
//  Serial.print("%");
//  Serial.print(" Moisture:");
//  Serial.print(moisture, 1); //mapping to max and min
//  Serial.println();
//
  //  Send to TS every 300 seconds
//  if (millis() > (lastDataSendTs + 3000000))
//  {
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, humd);
    ThingSpeak.setField(3, moisture);
    ThingSpeak.setField(4, 0);
    ThingSpeak.setField(5, 0);
    ThingSpeak.setField(6, 0);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
//    lastDataSendTs = millis();
//  }
//
//

    /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(2ULL * 60 * 60 * 1000 * 1000); //8 hrs
  Serial.println("Setup ESP32 to sleep for every " + String(2) +
  " Hours");

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
//  delay(10000);
}
