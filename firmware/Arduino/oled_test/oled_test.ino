/*

  Example-01_Hello.ino

  This demo shows the basic setup of the OLED library, generating simple graphics and displaying
  the results on the target device.

   Micro OLED             https://www.sparkfun.com/products/14532
   Transparent OLED       https://www.sparkfun.com/products/15173
   "Narrow" OLED          https://www.sparkfun.com/products/24606
   Qwiic OLED 1.3in       https://www.sparkfun.com/products/23453

  Written by Kirk Benell @ SparkFun Electronics, March 2022

  Repository:
     https://github.com/sparkfun/SparkFun_Qwiic_OLED_Arduino_Library

  Documentation:
     https://sparkfun.github.io/SparkFun_Qwiic_OLED_Arduino_Library/

  SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
*/

#include <SparkFun_Qwiic_OLED.h> //http://librarymanager/All#SparkFun_Qwiic_OLED

// The Library supports four different types of SparkFun boards. The demo uses the following
// defines to determine which device is being used. Uncomment the device being used for this demo.

// QwiicMicroOLED myOLED;
//QwiicTransparentOLED myOLED;
QwiicNarrowOLED myOLED;
//Qwiic1in3OLED myOLED;


void setup()
{
    Serial.begin(115200);
    Serial.println("Running OLED example");

    Wire.begin(14,15);

    // Initalize the OLED device and related graphics system
    if (myOLED.begin(Wire) == false)
    {
        Serial.println("Device begin failed. Freezing...");
        while (true)
            ;
    }
    Serial.println("Begin success");

    // Do a simple test - fill a rectangle on the screen and then print hello!

    // Fill a rectangle on the screen that has a 4 pixel board
    myOLED.rectangleFill(4, 4, myOLED.getWidth() - 8, myOLED.getHeight() - 8);

    String hello = "hello"; // our message

    // Center our message on the screen. Get the screen size of the "hello" string,
    // calling the getStringWidth() and getStringHeight() methods on the oled

    // starting x position - screen width minus string width  / 2
    int x0 = (myOLED.getWidth() - myOLED.getStringWidth(hello)) / 2;

    // starting y position - screen height minus string height / 2 
    int y0 = (myOLED.getHeight() - myOLED.getStringHeight(hello)) / 2;

    // Draw the text - color of black (0)
    myOLED.text(x0, y0, hello, 0);

    // There's nothing on the screen yet - Now send the graphics to the device
    myOLED.display();

    // That's it - HELLO!
}

void loop()
{ // There's nothing on the screen yet - Now send the graphics to the device
    myOLED.displayPower(true);
    delay(1000); // Do nothing
    myOLED.displayPower(false);
    delay(1000); // Do nothing

}
