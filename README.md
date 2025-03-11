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

## OSC configuration

1. **Configure WIFI connection**
    

## License

This project is licensed under the MIT License.
See the [LICENSE](https://github.com/Puara/magic-wand/tree/4-format-osc-messages-for-different-imu-data-types?tab=License-1-ov-file#) file for details.