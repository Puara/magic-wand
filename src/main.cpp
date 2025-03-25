//****************************************************************************//
// Puara based Magic Wand (with BNO055 + BMP280 IMU)                          //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

#include <EEPROM.h>
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

#define EEPROM_SIZE 32 // Adjust size based on your needs

#include <EEPROM.h>

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
        
// Create bundle to allow sending multiple OSC messages at once
OSCBundle bundle;

// Append message specific address
OSCMessage msgOrientation;
OSCMessage msgAcceleration;
OSCMessage msgGyroscope;

// Base address of OSC messages
std::string baseOSC;

// Offset orientation when calibrating
float yOffset;
float zOffset;

bool calibrationSaved = false; // Flag to track if calibration data has been saved

const bool calibrateOffset = true;

#define EEPROM_SIZE 32 // Adjust size based on your needs

void saveCalibration() {
    adafruit_bno055_offsets_t calibrationData;
    bno.getSensorOffsets(calibrationData);

    // Write calibration data to EEPROM
    EEPROM.put(0, calibrationData);
    EEPROM.commit();
    Serial.println("Calibration data saved to EEPROM.");
}

bool loadCalibration() {
    adafruit_bno055_offsets_t calibrationData;

    // Read calibration data from EEPROM
    EEPROM.get(0, calibrationData);

    // Check if the calibration data is valid
    if (calibrationData.accel_offset_x == 0xFFFF) {
        Serial.println("No valid calibration data found in EEPROM.");
        return false;
    }

    // Set the calibration data
    bno.setSensorOffsets(calibrationData);
    Serial.println("Calibration data loaded from EEPROM.");
    return true;
}

void checkAndSaveCalibration() {
    uint8_t systemCal, gyroCal, accelCal, magCal;
    bno.getCalibration(&systemCal, &gyroCal, &accelCal, &magCal);

    // Check if the system is fully calibrated
    if (systemCal == 3 && accelCal == 3 && magCal == 3) {
        Serial.println("System fully calibrated. Saving calibration data...");
        saveCalibration();
        Serial.println("Calibration data saved.");
    } else {
        Serial.print("Calibration in progress... ");
        Serial.print("Sys: ");
        Serial.print(systemCal);
        Serial.print(", Gyro: ");
        Serial.print(gyroCal);
        Serial.print(", Accel: ");
        Serial.print(accelCal);
        Serial.print(", Mag: ");
        Serial.println(magCal);
    }
}

float offsetValue(float currentValue, float  offsetAmount, float minValue, float maxValue){
    if(calibrateOffset){ // Keep original values if false
        currentValue -= offsetAmount;
    }
    //Loopback if value is outside defined range
    if( currentValue > maxValue){
        currentValue = minValue + (currentValue - maxValue);
    }
    if( currentValue < minValue){
        currentValue = maxValue - (minValue - currentValue);
    }
       return currentValue;
}

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
    Udp.begin(puara.LocalPORT());

    baseOSC = ("/" + puara.dmi_name()).c_str();
    
    // Set message specific address
    msgOrientation.setAddress((baseOSC + "/Orientation").c_str());
    msgAcceleration.setAddress((baseOSC + "/Acceleration").c_str());
    msgGyroscope.setAddress((baseOSC + "/Gyroscope").c_str());

    //=== turn on and init the tft screen ===
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH);
    tft.init(135, 240); // Init ST7789 240x135
    tft.setRotation(3); // rotates the screen

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);

    //=== init BNO IMU ===
    if (!bno.begin(OPERATION_MODE_COMPASS)) { // Change to compass mode
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.println("No BNO055 detected... Check your wiring or I2C ADDR!");
        while (1)
        ;
    }

    bno.setExtCrystalUse(true);

    // Load calibration data from EEPROM
    if (!loadCalibration()) {
        Serial.println("No valid calibration data found in EEPROM. Please calibrate the sensor.");
    } else {
        Serial.println("Calibration data successfully loaded from EEPROM.");

        // Verify calibration status
        uint8_t systemCal, gyroCal, accelCal, magCal;
        bno.getCalibration(&systemCal, &gyroCal, &accelCal, &magCal);

        if (systemCal < 3 || accelCal < 3 || magCal < 3) {
            Serial.println("Loaded calibration data is incomplete. Please recalibrate the sensor.");
            
            // Wait for calibration and save it
            while (true) {
                checkAndSaveCalibration();
                delay(1000); // Check calibration status every second

                // Save calibration data and break the loop if fully calibrated
                if (bno.isFullyCalibrated()) {
                    saveCalibration(); // Ensure calibration data is saved
                    break;
                }
            }
        } else {
            Serial.println("Loaded calibration data is valid.");
        }
    }
    
    delay(1000);
    sensors_event_t initialOrientation;
    bno.getEvent(&initialOrientation, Adafruit_BNO055::VECTOR_EULER);
    yOffset = initialOrientation.orientation.y;
    zOffset = initialOrientation.orientation.z;


    Serial.println("setup completed successfully");

}

void loop() {

    /* Get a new event per sensor */
    sensors_event_t orientationData, /* angVelocityData, */ accelerometerData, magneticData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    // bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE); // Commented out gyroscope data
    bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&magneticData, Adafruit_BNO055::VECTOR_MAGNETOMETER);

    // Calculate heading (angle relative to magnetic north)
    float heading = atan2(magneticData.magnetic.y, magneticData.magnetic.x) * (180.0 / PI);

    // Normalize heading to [0, 360]
    if (heading < 0) {
        heading += 360;
    }

    // Get calibration status
    uint8_t systemCal, gyroCal, accelCal, magCal;
    bno.getCalibration(&systemCal, &gyroCal, &accelCal, &magCal);

    /* 
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the 
     * network (WiFiUdp will print an warning message in those cases).
     */
    if (puara.IP1_ready()) { // set namespace and send OSC message for address 1
    
        //bundle.add(msgOrientation.add(orientationData.orientation.x).add(offsetValue(orientationData.orientation.y, yOffset, -180, 180)).add(offsetValue(orientationData.orientation.z, zOffset, -90, 90)));
        bundle.add(msgAcceleration.add(accelerometerData.acceleration.x).add(accelerometerData.acceleration.y).add(accelerometerData.acceleration.z));
        // bundle.add(msgGyroscope.add(angVelocityData.acceleration.x).add(angVelocityData.acceleration.y).add(angVelocityData.acceleration.z)); // Commented out gyroscope OSC message
        
        Udp.beginPacket(puara.IP1().c_str(), puara.PORT1());
        bundle.send(Udp);
        Udp.endPacket();

        Udp.beginPacket(puara.IP2().c_str(), puara.PORT2());
        bundle.send(Udp);
        Udp.endPacket();

        // Clear OSC 
        bundle.empty();
        msgOrientation.empty();
        msgAcceleration.empty();
        // msgGyroscope.empty(); // Commented out gyroscope message clearing
    }

    /* Display the floating point orientation data and IP address */
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setCursor(0, 10);
    canvas.setTextColor(ST77XX_MAGENTA);
    canvas.setTextSize(2);
    canvas.print("X: ");
    canvas.print(orientationData.orientation.x, 4);
    canvas.setTextColor(ST77XX_WHITE);
    canvas.print("\nY: ");
    canvas.print((offsetValue(orientationData.orientation.y, yOffset, -90, 90)), 4);
    canvas.setTextColor(ST77XX_CYAN);
    canvas.print("\nZ: ");
    canvas.print((offsetValue(orientationData.orientation.z, zOffset, -180, 180)), 4);
    canvas.print("\nIP: ");
    canvas.print(puara.staIP().c_str());
    canvas.print("\nHeading: ");
    canvas.print(heading, 2); // Display the calculated heading
    canvas.print("\nCalib: ");
    canvas.print("Sys:");
    canvas.print(systemCal);
    canvas.print(" G:");
    canvas.print(gyroCal);
    canvas.print(" A:");
    canvas.print(accelCal);
    canvas.print(" M:");
    canvas.print(magCal); // Display calibration status for magnetometer
    
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 135);
    
    delay(10);
}