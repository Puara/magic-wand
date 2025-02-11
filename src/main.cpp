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

// ======== PUARA ========
#include "Arduino.h"
#include "puara.h"
Puara puara;

// ======== OSC ========

#include <WiFiUdp.h>
#include <OSCMessage.h>
WiFiUDP Udp;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);

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

  /* OSC test*/

  if(puara.IP1_ready()){
    OSCMessage msg1(("/" + puara.get_dmi_name()).c_str());
    float testValue = 0.5;
    msg1.add(testValue);
    Udp.beginPacket(puara.getIP1().c_str(), puara.getPORT1());
    msg1.send(Udp);
    Udp.endPacket();
    msg1.empty();
  }

  /* Display the floating point data */
  canvas.fillScreen(ST77XX_BLACK);
  canvas.setCursor(0, 10);
  canvas.setTextColor(ST77XX_MAGENTA);
  canvas.setTextSize(3);
  canvas.print("X: ");
  canvas.print(event.orientation.x, 4);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.print("\nY: ");
  canvas.print(event.orientation.y, 4);
  canvas.setTextColor(ST77XX_CYAN);
  canvas.print("\nZ: ");
  canvas.print(event.orientation.z, 4);
  canvas.println("");

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 135);
  delay(10);
}
