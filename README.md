# 🌱 Zyro — Smart Agriculture Monitoring System

An IoT-based smart irrigation and soil monitoring system built on **ESP32** with real-time data visualization and remote control via the **Blynk IoT dashboard**.

---

## 📌 Overview

Zyro is a smart agriculture project designed to automate and monitor plant irrigation using multiple environmental sensors. The system collects real-time data on soil moisture, temperature, humidity, rain, and soil pH — and uses that data to automatically control a water pump. The farmer can also monitor all sensor readings and manually override the pump from anywhere using the Blynk mobile app.

---

## ✨ Features

- 🌊 **Auto Irrigation** — Pump turns ON/OFF automatically based on soil moisture and temperature
- 🌧️ **Rain Detection** — Stops watering automatically when rain is detected (DO pin)
- 📊 **Rain Intensity Measurement** — Measures how much it is raining using analog output (AO pin)
- 🧪 **Soil pH Monitoring** — Detects whether soil is acidic, optimal, or alkaline
- 📱 **Blynk Dashboard** — View all sensor data in real time from your phone
- 🔧 **Manual Pump Control** — Override auto mode and control the pump directly from Blynk
- 🔄 **Auto/Manual Mode Toggle** — Switch between automatic and manual control anytime
- 🔁 **State Sync on Reconnect** — Restores pump and mode state after WiFi dropout

---

## 🛠️ Hardware Used

| Component | Purpose |
|-----------|---------|
| ESP32 | Main microcontroller + WiFi |
| Capacitive Soil Moisture Sensor | Measures soil moisture % |
| DS18B20 | Soil/water temperature |
| DHT11 | Air temperature & humidity |
| Rain Sensor Module | Rain detection (DO) + Rain intensity (AO) |
| pH Sensor (E-201-C) | Measures soil pH level |
| Relay Module (1-channel) | Controls the water pump |
| DC Water Pump / Motor | Irrigation water delivery |

---

## 📡 ESP32 Pin Configuration

| GPIO   |      Component      |        Signal         |
|--------|---------------------|-----------------------|
| GPIO34 | Soil Moisture Sensor| Analog Output         |
| GPIO4  | DS18B20 Temperature | OneWire Data          |
| GPIO5  | DHT11               | Digital Data          |
| GPIO27 | Rain Sensor         | DO (Digital detect)   |
| GPIO33 | Rain Sensor         | AO (Analog intensity) |
| GPIO32 | pH Sensor (E-201-C) | Analog Output         |
| GPIO14 | Relay Module        | IN (Pump control)     |

---

## 📲 Blynk Dashboard — Virtual Pins

| Virtual Pin | Widget | Label | Description |
|-------------|--------|-------|-------------|
| V0 | Gauge | Soil Moisture | 0–100% |
| V1 | Gauge | Temperature | °C from DS18B20 |
| V2 | Gauge | Air Humidity | % from DHT11 |
| V3 | LED | Rain Status | DRY / RAINING |
| V4 | LED | Pump Status | ON / OFF indicator |
| V5 | Switch | Mode | Auto (0) / Manual (1) |
| V6 | Button (Switch) | Pump Control | Manual ON/OFF |
| V7 | Gauge | Soil pH | 0.0 – 14.0 |
| V8 | Value Display | pH Status | Acidic / Optimal / Alkaline |
| V9 | Gauge | Rain Intensity | 0–100% |

---

## ⚙️ Auto Irrigation Logic

```
IF rain detected (DO = LOW)
    → Pump OFF (no watering needed)

ELSE IF soil moisture < 30%
    → Pump ON (soil is dry)

ELSE IF soil moisture > 70%
    → Pump OFF (soil is wet enough)

ELSE IF temperature > 35°C AND soil moisture < 40%
    → Pump ON (hot day, extra watering)
```

Manual mode completely bypasses the above logic — the pump responds only to the Blynk button.

---

## 🧪 pH Sensor Calibration

The E-201-C pH sensor requires two-point calibration using buffer solutions:

1. Dip sensor in **pH 7.0** buffer solution → note the raw ADC value
2. Dip sensor in **pH 4.0** buffer solution → note the raw ADC value
3. Calculate:
```
SLOPE     = (voltage_at_4 - voltage_at_7) / (4.0 - 7.0)
INTERCEPT = 7.0 - (SLOPE * voltage_at_7)
```
4. Update these values in the code:
```cpp
const float PH_SLOPE     = -1.8766;  // your value
const float PH_INTERCEPT =  9.196367; // your value
```

---

## 🌱 Soil Moisture Sensor Calibration

The capacitive sensor returns a raw ADC value. Calibrate by:

1. Reading the sensor **in open air** (completely dry) → set as `SOIL_RAW_DRY`
2. Reading the sensor **submerged in water** → set as `SOIL_RAW_WET`

```cpp
const int SOIL_RAW_DRY = 2800; // your dry value
const int SOIL_RAW_WET = 1200; // your wet value
```

---

## 📦 Libraries Required

Install these from Arduino Library Manager:

| Library | Purpose |
|---------|---------|
| `BlynkESP32` | Blynk IoT connectivity |
| `DHT sensor library` (Adafruit) | DHT11 sensor |
| `OneWire` | OneWire protocol for DS18B20 |
| `DallasTemperature` | DS18B20 temperature reading |

---

## 🚀 Getting Started

1. Clone this repository
```bash
git clone https://github.com/yourusername/zyro-smart-agriculture.git
```

2. Open `zyro.ino` in Arduino IDE

3. Install all required libraries listed above

4. Update your credentials in the code:
```cpp
#define BLYNK_TEMPLATE_ID   "your_template_id"
#define BLYNK_TEMPLATE_NAME "Zyro"
#define BLYNK_AUTH_TOKEN    "your_auth_token"

const char* ssid     = "your_wifi_name";
const char* password = "your_wifi_password";
```

5. Calibrate soil moisture and pH sensors as described above

6. Select **ESP32 Dev Module** as board in Arduino IDE

7. Upload and open Serial Monitor at **115200 baud** to verify readings

8. Set up the Blynk dashboard with the virtual pins listed above

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/830b215d-22b1-4329-8da5-bebcaff4a9d1" />

---

## 📊 Serial Monitor Output Format

```
[Soil:45%] [Temp:28.5°C] [Hum:62.0%] [Rain:NO | Intensity:0%] [pH:6.72 | Optimal pH] [Pump:AUTO | OFF]
```

---

## 📁 Project Structure

```
zyro-smart-agriculture/
│
├── zyro.ino               # Main Arduino sketch
├── README.md              # Project documentation
└── assets/
    └── dashboard.png      # Blynk dashboard screenshot (optional)
```

---

## 🔮 Future Improvements

- [ ] Add NPK sensor for complete soil nutrition monitoring
- [ ] Data logging to Google Sheets via webhooks
- [ ] Blynk notifications/alerts when soil is critically dry
- [ ] Solar power integration
- [ ] Water flow sensor for usage tracking
- [ ] Multi-zone irrigation support
- [ ] Crop Disease Detection by image processing

---


## 👨‍💻 Author

**Soumyadeep** <br>
**Archisman**   <br>
**Subhankar**<br>
**Sujauy**<br>
**KGEC,1st year**
Built with ❤️ using ESP32 + Blynk IoT

Web  Dashboard

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
