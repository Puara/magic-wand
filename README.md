# SAT's Magic Wand

This demonstrates the use of an [ESP32-S3 Feather](https://learn.adafruit.com/adafruit-esp32-s3-tft-feather/overview) microcontroller and a [BNO055](https://www.adafruit.com/product/5937) sensor. 
The ESP32 reads data from the BNO055 sensor and sends it via OSC, integrating Puara's [basic-template](https://github.com/Puara/puara-module-templates/tree/main/basic-osc).
The IMU data is also displayed on its TFT screen.

## Setup

1. **Clone the repository:**
    ```sh
    git clone https://github.com/Puara/magic-wand.git
    ```
2. **Install requirements**: [PlatformIO](https://platformio.org/install) + [Wokwi](https://docs.wokwi.com/) CLIs / VSCode extensions
3. **Open as PlatformIO project:**
    Build, upload, and monitor results.

    <img width="460" alt="Screenshot 2025-02-11 at 2 28 12 PM" src="https://github.com/user-attachments/assets/8d71eaf4-7062-474b-886c-80eb8f0d25e3" />

4. **Output:**
    Monitor the output on the TFT screen and the serial output using PlatformIO with the monitor option.

    <img width="740" alt="Screenshot 2025-02-11 at 2 36 36 PM" src="https://github.com/user-attachments/assets/34e9b6b1-7bd3-4b03-8537-10d9ef50bf87" />

    or the simulation with Wokwi, by running the [diagram.json](https://github.com/Puara/magic-wand/blob/main/diagram.json) (this is a WIP)

    <img width="763" alt="Screenshot 2025-02-11 at 2 30 13 PM" src="https://github.com/user-attachments/assets/48288e7e-deaa-42c0-891e-386f7648f610" />

## Configure WiFi connection

1. **Connect to the device**
    
    On startup, the device should broadcast its own WiFi access point named "(device name)_(device id)", as specified in the [config.json](data\config.json) file.
    The default name for this access point should appear as MagicWand_001 and the password can be set in the config file.

2. **Connect the device to your own WiFi network** (Optional)

    Once connected to the access point, a web interface can be accessed at the address MagicWand_001.local or by typing in the device's IP address.
    On this page, enter your SSID and network password and reboot the device.
    If done correctly, the IP address on the screen should change.

3. **Add output IP address**

    The device's web interface also allows specifying up to 2 output addresses and ports.
    
## IMU Calibration

**Initial calibration**

When booting, the device calibrates to its current orientation.
If recalibration of the horizontal axis is needed, a pulse can be sent to the OSC adress **/reCalibrate**.

## License

This project is licensed under the MIT License.
See the [LICENSE](https://github.com/Puara/magic-wand/tree/4-format-osc-messages-for-different-imu-data-types?tab=License-1-ov-file#) file for details.