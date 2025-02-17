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
#include <OSCBundle.h>

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
    sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
    bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

    /* 
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the 
     * network (WiFiUdp will print an warning message in those cases).
     */
    if (puara.IP1_ready()) { // set namespace and send OSC message for address 1
        OSCBundle bundle;

        String baseMessage = ("/" + puara.get_dmi_name()).c_str();

        OSCMessage msgOrientation((baseMessage + "/Orientation").c_str());
        OSCMessage msgAcceleration((baseMessage + "/Acceleration").c_str());
        OSCMessage msgGyroscope((baseMessage + "/Gyroscope").c_str());
        //msg1.add(event.orientation.x).add(event.orientation.y).add(event.orientation.z);
        
        msgOrientation.add(orientationData.orientation.x).add(orientationData.orientation.y).add(orientationData.orientation.z);
        msgAcceleration.add(accelerometerData.acceleration.x).add(accelerometerData.acceleration.y).add(accelerometerData.acceleration.z);
        msgGyroscope.add(angVelocityData.acceleration.x).add(angVelocityData.acceleration.y).add(angVelocityData.acceleration.z);

        bundle.add(msgOrientation);
        bundle.add(msgAcceleration);
        bundle.add(msgGyroscope);
        
        Udp.beginPacket(puara.getIP1().c_str(), puara.getPORT1());
        bundle.send(Udp);
        Udp.endPacket();
        msgOrientation.empty();
        msgAcceleration.empty();
        msgGyroscope.empty();
    }

    /* Display the floating point data */
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setCursor(0, 10);
    canvas.setTextColor(ST77XX_MAGENTA);
    canvas.setTextSize(3);
    canvas.print("X: ");
    canvas.print(orientationData.orientation.x, 4);
    canvas.setTextColor(ST77XX_WHITE);
    canvas.print("\nY: ");
    canvas.print(orientationData.orientation.y, 4);
    canvas.setTextColor(ST77XX_CYAN);
    canvas.print("\nZ: ");
    canvas.print(orientationData.orientation.z, 4);
    
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 135);
    
    delay(10);
}
