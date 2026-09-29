# 🤖 ESPThy: Standalone RGB Meta-Sensor for Thymio II

This project extends the hardware capabilities of the **Thymio II educational robot** by enabling it to recognize and react to RGB colors emitted by backlit screens (LCD/OLED monitors, interactive tables). 

Since the native infrared (IR) sensors of the Thymio are blinded by the glass of backlit screens, this project bypasses the limitation using an **ESP32** and a **TCS34725 RGB Sensor** acting as an autonomous "Meta-Sensor".

## 🚀 Two Architectures Explored

### 1. Standalone RC5 Architecture (Definitive Meta-Sensor)
*Folders: `esp32_firmware/` and `thymio_aseba/`*
The ESP32 reads the raw RGB values, filters optical noise (screen flickering, subpixel reflection) using a **Majority Vote Statistical Filter**, and computes the dominant color. It then transmits the corresponding action directly to the Thymio's native IR receiver using the **RC5 protocol**, creating a 100% wireless, PC-independent mechatronic system. An **Anti-Flooding** logic ensures the robot's buffer is not overloaded.

### 2. Wi-Fi / UDP Bridge Architecture (Legacy)
*Folder: `python_bridge_legacy/`*
An IoT-based approach where the ESP32 acts as a wireless edge-sensor. It samples the color and sends a UDP datagram over the local Wi-Fi network to a Python Host PC. The PC translates the string and remotely controls the Thymio via the `tdmclient` API.

## 🛠️ Hardware Requirements
* **Thymio II Robot**
* **ESP32** Microcontroller (e.g., M5Stack or standard NodeMCU)
* **TCS34725** I2C RGB Color Sensor
* **Infrared LED** (940nm) & Resistor
* 5V Powerbank (to power the ESP32 array)

## ⚙️ Setup & Usage (Standalone Mode)
1. **ESP32 Setup:** Flash the `esp32_firmware/meta_sensor_v2.ino` code to your ESP32. Ensure the IR LED is connected to Pin 4 and GND.
2. **Thymio Setup:** Open Aseba Studio, connect the Thymio, and load the `thymio_aseba/reattivita_motori.aesl` script.
3. **Mounting:** Attach the ESP32 and sensor to the Thymio, pointing the IR LED directly at the Thymio's front IR receiver.
4. **Action:** Place the robot on an active RGB screen. It will now autonomously stop on Red/Black, move forward on Green, and rotate on Blue!

## 📄 Documentation
For an in-depth analysis of the physical limitations of native IR sensors on glass, the calibration process, and the mathematical logic behind the Majority Vote filter, please refer to the detailed report in the `docs/` folder.
