//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//


#include "Arduino.h"
// ======== tft screen ========
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);

// ======== BNO IMU ========
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <utility/imumaths.h>
Adafruit_BNO055 bno = Adafruit_BNO055();


// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"

#include <iostream>

// Initialize Puara's module manager
Puara puara;

/*
 * Include CNMAT's OSC library (by Adrian Freed)
 * This library was chosen as it is widely used, but it can be replaced by any
 * other OSC library of choice
 */
#include <WiFiUdp.h>
#include <OSCMessage.h>

// UDP instances to let us send and receive packets
WiFiUDP Udp;

// Dummy sensor data
float sensor;

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    /*
     * the Puara start function initializes the spiffs, reads config and custom json
     * settings, start the wi-fi AP/connects to SSID, starts the webserver, serial 
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();

    // Start the UDP instances 
    Udp.begin(puara.getLocalPORT());

    //=== turn on and init the tft screen ===
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH);
    tft.init(135, 240); // Init ST7789 240x135
    tft.setRotation(3); // rotates the screen

    //=== init BNO IMU ===
    if (!bno.begin(OPERATION_MODE_IMUPLUS)) {
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.println("No BNO055 detected... Check your wiring or I2C ADDR!");
        while (1)
        ;
    }

    delay(1000);

    bno.setExtCrystalUse(true);
    Serial.println("setup completed successfully");

}

void loop() {

    /* Get a new sensor event */
    sensors_event_t event;
    bno.getEvent(&event);

    std::string baseMessage = ("/Magic_Wand");

    /* 
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the 
     * network (WiFiUdp will print an warning message in those cases).
     */
    if (puara.IP1_ready()) { // set namespace and send OSC message for address 1
        OSCMessage msgOrient((baseMessage + "/Orientation").c_str());
        OSCMessage msgAccl((baseMessage + "/Accelerometer").c_str());
        msgOrient.add(event.orientation.x).add(event.orientation.y).add(event.orientation.z);
        //msgAccl.add(event.orientation.x).add(event.orientation.y).add(event.orientation.z);
        //msgAccl.add(event.acceleration.x).add(event.acceleration.y).add(event.acceleration.z);
        Udp.beginPacket(puara.getIP1().c_str(), puara.getPORT1());
        msgOrient.send(Udp);
        msgAccl.send(Udp);
        Udp.endPacket();
        msgOrient.empty();
        msgAccl.empty();
    }

    /* Display the floating point data */
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setCursor(0, 10);
    canvas.setTextColor(ST77XX_MAGENTA);
    canvas.setTextSize(2);
    canvas.print("X: ");
    canvas.print(event.orientation.x, 4);
    canvas.setTextColor(ST77XX_WHITE);
    canvas.print("\nY: ");
    canvas.print(event.orientation.y, 4);
    canvas.setTextColor(ST77XX_CYAN);
    canvas.print("\nZ: ");
    canvas.print(event.orientation.z, 4);
    /*canvas.print("\nIP: ");
    canvas.print();*/
    
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 135);
    
    delay(10);
}
