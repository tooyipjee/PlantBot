#include "../../pins_defs.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <Wire.h>
#define LEDS_COUNT  3
#define CHANNEL		0
void scanI2C();
#include <BLEDevice.h>
#include <BLEAdvertising.h>

esp_ble_gap_ext_adv_params_t ext_adv_params_2M = {
  .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_NONCONN_NONSCANNABLE_UNDIRECTED,
  .interval_min = 0x40,
  .interval_max = 0x40,
  .channel_map = ADV_CHNL_ALL,
  .own_addr_type = BLE_ADDR_TYPE_RANDOM,
  .peer_addr_type = BLE_ADDR_TYPE_RANDOM,
  .peer_addr = {0, 0, 0, 0, 0, 0},
  .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
  .tx_power = EXT_ADV_TX_PWR_NO_PREFERENCE,
  .primary_phy = ESP_BLE_GAP_PHY_1M,
  .max_skip = 0,
  .secondary_phy = ESP_BLE_GAP_PHY_2M,
  .sid = 1,
  .scan_req_notif = false,
};

static uint8_t raw_scan_rsp_data_2m[] = {0x02, 0x01, 0x06, 0x02, 0x0a, 0xeb, 0x12, 0x09, 'E', 'S', 'P', '_', 'M',
                                         'U',  'L',  'T',  'I',  '_',  'A',  'D',  'V',  '_', '2', 'M', 0X0};

static esp_ble_gap_periodic_adv_params_t periodic_adv_params = {
  .interval_min = 0x320,  // 1000 ms interval
  .interval_max = 0x640,
  .properties = 0,  // Do not include TX power
};

static uint8_t periodic_adv_raw_data[] = {0x02, 0x01, 0x06, 0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd, 0x11, 0x09, 'E', 'S',
                                          'P',  '_',  'P',  'E',  'R',  'I',  'O',  'D',  'I',  'C',  '_',  'A',  'D', 'V'};

uint8_t addr_2m[6] = {0xc0, 0xde, 0x52, 0x00, 0x00, 0x02};

BLEMultiAdvertising advert(1);  // max number of advertisement data

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LED_DAT, CHANNEL, TYPE_GRB);
int currentMillis = 0;
void setup() {
  strip.begin();
  strip.setBrightness(10);  
  Serial.begin(115200);
  Serial.println("Multi-Advertising...");
  
  BLEDevice::init("");

  advert.setAdvertisingParams(0, &ext_adv_params_2M);
  advert.setAdvertisingData(0, sizeof(raw_scan_rsp_data_2m), &raw_scan_rsp_data_2m[0]);
  advert.setInstanceAddress(0, addr_2m);
  advert.setDuration(0, 0, 0);

  delay(100);
  advert.start();
  advert.setPeriodicAdvertisingParams(0, &periodic_adv_params);
  advert.setPeriodicAdvertisingData(0, sizeof(periodic_adv_raw_data), &periodic_adv_raw_data[0]);
  advert.startPeriodicAdvertising(0);

  currentMillis = millis();

  // Setting R,G,Y LEDs 
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);

  // Set up BAT and LED_PWR
  pinMode(BAT, OUTPUT);
  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  // Set up READ_BATT
  pinMode(READ_BATT, INPUT);
  pinMode(CAP_SIG, INPUT);

  // I2C setup
  Wire.begin(SDA,SCL);

}

void loop() {
  for (int j = 0; j < 255; j += 2) {
    for (int i = 0; i < LEDS_COUNT; i++) {
      strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + j) & 255));
    }
    strip.show();
    delay(10);
  }  
  if(millis()>currentMillis+2000)
  {
    currentMillis = millis();
    Serial.println("tock");
    // R,G,Y LEDs
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);

    // Turn on batt and read batt;
    digitalWrite(BAT, HIGH);
    Serial.print("Batt raw: "); 
    Serial.print(analogRead(READ_BATT));
    Serial.print(", Cap sig: "); 
    Serial.println(analogRead(CAP_SIG));
    // digitalWrite(LED_PWR, LOW);



  }
  else if(millis()>currentMillis+1000)
  {
    Serial.println("tick");

    // R,G,Y LEDs
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(YELLOW_PIN, HIGH);
    // digitalWrite(LED_PWR, HIGH);

    digitalWrite(BAT, LOW);
    scanI2C();

  }



  delay(10);
}




void scanI2C()
{
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
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
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}