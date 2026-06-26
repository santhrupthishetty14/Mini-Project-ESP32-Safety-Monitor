
---

# 📡 ESP32 Safety Monitoring System

**Heart Rate + Temperature + GPS Tracking + SMS Alerts (TTGO T-Call ESP32 SIM800L)**
*A Personal Safety / Health Monitoring Device with Cellular Connectivity*

---

## 🧭 Overview

This project uses the **TTGO T-Call ESP32** with built-in **SIM800L GSM module** to create a portable safety/health monitoring system. It reads:

* **Heart rate** using a Pulse Sensor (HW-827)
* **Body temperature** using **DS18B20 digital temperature sensor**
* **GPS coordinates** using **NEO-6M GPS module**
* Sends **SMS alerts** via GSM (SIM800L)
* Can integrate with **Blynk IoT** for live monitoring

The device performs a 10-second heart rate test, estimates BPM, checks abnormalities, and sends the results via SMS.
Further stages include continuous monitoring + Blynk + GPS tracking.

---

## ✨ Features

* ❤️ **Heart rate measurement** using filtered ADC waveform
* 🌡 **Temperature reading** through DS18B20
* 📍 **GPS location** using NEO-6M (GY-GPS6MV2)
* 📲 **SMS sending** using SIM800L (AT command mode)
* 📡 Optional **Blynk IoT integration** for live dashboard
* 🔋 Works on battery or 5V supply
* 🚨 **Automatic alerts** for abnormal health readings

---

## 🔧 Hardware Required

| Component                     | Description                                  |
| ----------------------------- | -------------------------------------------- |
| **TTGO T-Call ESP32 SIM800L** | ESP32 board with built-in SIM800L GSM module |
| **Pulse Sensor HW-827**       | Photoplethysmography heart-beat sensor       |
| **DS18B20**                   | Digital waterproof temperature sensor        |
| **NEO-6M GPS (GY-GPS6MV2)**   | GPS module with ceramic antenna              |

---

## 🔌 Wiring Diagram

![Wiring Diagram for the Safety Monitoring System](wiring-diagram.png)

### **📡 TTGO T-Call ESP32 Pin Mapping**

### 🩺 Pulse Sensor (HW-827)

| Pulse Sensor | TTGO Pin |
| ------------ | -------- |
| VCC          | 3.3V     |
| GND          | GND      |
| OUT          | GPIO 35  |

---

### 🌡 DS18B20 Temperature Sensor

| DS18B20 Pin    | TTGO Pin          |
| -------------- | ----------------- |
| VCC            | 3.3V              |
| GND            | GND               |
| DQ             | GPIO 13           |
| 4.7kΩ resistor | Between DQ & 3.3V |

---

### 📍 GPS Module (NEO-6M / GY-GPS6MV2)

| GPS Pin | TTGO Pin |
| ------- | -------- |
| VCC     | 5V       |
| GND     | GND      |
| TX      | GPIO 34  |
| RX      | GPIO 25  |

---



## 🖥 ASCII Wiring Block Diagram

```
                  +---------------------------+
                  |     TTGO T-Call ESP32     |
                  |   (ESP32 + SIM800L GSM)   |
     3.3V ----+---- DS18B20 VCC               |
              |         GPIO13 <--- DS18B20 DQ|---[4.7kΩ]--- +3.3V
              |                                |
              +---- Pulse VCC                  |
              |         GPIO35 <--- Pulse OUT  |
              |                                |
     5V  ---->+---- GPS VCC                    |
 GND ---------+---- GND (ALL MODULES)          |
              |  GPS TX ---> GPIO34 (RX2)      |
              |  GPS RX <--- GPIO25 (TX2)      |
              |              |
              +---------------------------+
```

---

## 📦 Software Overview

### Main Functional Blocks

* **Pulse measurement (10 sec)** → filtered ADC → BPM calculation
* **Temperature reading** (future stage)
* **SIM800L SMS communication** using AT commands
* **GPS parsing** with TinyGPS++ (future stage)
* **Blynk integration** (future stage)

---

## 🛠 Code Structure

```
/project
 ├── src/
 │   ├── pulse_test_sms.ino   # Heart rate measurement + SMS
 │   ├── gps_test.ino         # GPS + Serial monitor
 │   ├── temp_test.ino        # DS18B20 test
 │   └── full_system.ino      # Final integrated code
 ├── README.md                # This file
 ├── wiring-diagram.png       # Optional image
 └── LICENSE                  # Optional
```

---

## 🚀 How to Run

1. Install **Arduino IDE** or **PlatformIO**
2. Install required libraries:

   * TinyGPS++
   * OneWire
   * DallasTemperature
   * TinyGSM (if using Blynk later)
3. Select:

   * **Board:** ESP32 Dev Module
   * **Port:** USB COM port
4. Upload the code
5. Open **Serial Monitor at 115200 baud**
6. Follow the instructions (place finger → countdown → BPM result)
7. Device automatically sends SMS

---

## 🧪 Usage Example

```
Place your finger gently on the pulse sensor.
Starting measurement in 5 seconds...
Beat #1 at 450 ms
Beat #2 at 1240 ms
...
Measurement finished.

Your heart rate is approximately: 76.2 bpm  
Sending SMS...
```

---

## 🔮 Future Enhancements

* Live monitoring in **Blynk IoT app**
* GPS tracking + map view
* Auto-alert if:

  * HR too high/low
  * Temperature abnormal
  * No movement detected
* SOS button to send instant alert
* Small OLED screen UI
* Battery level reporting

---

## 📜 License

MIT License — feel free to use, modify, distribute.

---
