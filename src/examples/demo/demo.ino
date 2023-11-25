#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <WiFiClientSecure.h>

// RemoteXY configuration for ESP32 with BLE (Bluetooth Low Energy) support
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// Define the name for the Bluetooth connection
#define REMOTEXY_BLUETOOTH_NAME "Plant-Bot"

// RemoteXY configuration data
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { /* Configuration data goes here */ };
#pragma pack(pop)

// This structure defines the variables and events for the RemoteXY interface
struct {
    // Input variables
    int8_t Threshold = 50; // Slider position for threshold, range 0-100
    uint8_t PumpDirection; // Switch status: 1 = ON, 0 = OFF

    // Output variables
    int8_t Light; // Light level, range 0-100
    int8_t Moisture; // Moisture level, range 0-100

    // Other variables
    uint8_t connect_flag;  // Connection flag: 1 = connected, 0 = not connected
} RemoteXY;

// Pin definitions for various components
#define LIGHT_SIG 0
#define LIGHT_PWR 2
#define MOISTURE_SIG 4
#define IND_LED 5
#define MOTOR_IN1 6
#define MOTOR_IN2 7
#define WS2812 10

#define LEDS_COUNT 1

// Initialize the WS2812 LED strip
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, WS2812, 0);
bool messageSent = false;
bool timedOut = false;
int timeWatered = millis();

// Function declarations
void update();

// Setup function, runs once at startup
void setup() {
    RemoteXY_Init();
    Serial.begin(115200); // Initialize serial communication
    strip.begin(); // Initialize the LED strip
    strip.setBrightness(10); // Set brightness of the LED strip

    // Set pin modes for various components
    pinMode(LIGHT_SIG, INPUT);
    pinMode(MOISTURE_SIG, INPUT);
    pinMode(IND_LED, OUTPUT);
    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(LIGHT_PWR, OUTPUT);

    digitalWrite(LIGHT_PWR, HIGH); // Turn on the light
}

// Main loop function, runs repeatedly
void loop() {
    update();

    // Moisture control logic
    if(RemoteXY.Moisture < RemoteXY.Threshold && !timedOut) {
        digitalWrite(IND_LED, HIGH);
        if(millis() > timeWatered + 60000) {
            timedOut = true;
        }
    } else {
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        digitalWrite(IND_LED, LOW);
        timeWatered = millis();
    }

    // Pump control logic
    if(RemoteXY.PumpDirection) {
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
    } else {
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
    }

    // Send a message if the system is out of water
    if(!messageSent && timedOut) {
        // Code for sending message (commented out)
        messageSent = true;
    }
}

// Update function for handling sensor readings and RemoteXY communication
void update() {
    RemoteXY_Handler();
    RemoteXY.Light = analogRead(LIGHT_SIG) / 40.96;
    delay(10);

    // Soil moisture sensor reading
    RemoteXY.Moisture = 100 - 100.0 * (analogRead(MOISTURE_SIG) - 1800) / (2700 - 1800);
    if (abs(RemoteXY.Moisture) > 1000 || RemoteXY.Moisture < 1) {
        RemoteXY.Moisture = 0;
    }
    Serial.print("Moist.: ");
    Serial.print(RemoteXY.Moisture);
    Serial.print(", Light: ");
    Serial.println(RemoteXY.Light);

    // Neopixel LED color control
    int col = int(RemoteXY.Moisture * 160 / 100);
    strip.setLedColorData(0, strip.Wheel((0 / 1 + col) & 255));
    strip.show();
}
