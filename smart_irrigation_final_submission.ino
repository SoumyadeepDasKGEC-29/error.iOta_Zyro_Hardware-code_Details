#define BLYNK_TEMPLATE_ID   "TMPL3Pp5FeOj0"
#define BLYNK_TEMPLATE_NAME "Zyro"
#define BLYNK_AUTH_TOKEN    "edDNyhLlRo4VPzEkb2TVe_fMBOD1jMHR"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid     = "Soumyadeep's A06";
const char* password = "200620554";

// ── Pins ────────────────────────────────────────────────────────────────────
#define SOIL_MOISTURE_PIN   34
#define DS18B20_PIN          4
#define DHT_PIN              5
#define RAIN_SENSOR_PIN     27
#define RELAY_PUMP_PIN      14
#define PH_PIN              32
#define RAIN_SENSOR_AO_PIN  33   // AO pin → GPIO35


// --- pH Callibration ________________________________________________________

const float SLOPE     = -1.8766;
const float INTERCEPT =  9.196367;
const int   SAMPLES   = 10;

// ── Sensor objects ───────────────────────────────────────────────────────────
DHT dht(DHT_PIN, DHT11);
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

// ── Thresholds ───────────────────────────────────────────────────────────────
const int   SOIL_DRY_THRESHOLD  = 30;   // % — pump ON below this
const int   SOIL_WET_THRESHOLD  = 70;   // % — pump OFF above this
const float TEMP_HIGH_THRESHOLD = 35.0; // °C — water more aggressively

// ── Soil calibration (adjust to your sensor) ─────────────────────────────────
const int SOIL_RAW_DRY = 2800;
const int SOIL_RAW_WET = 1200;

// ── State ─────────────────────────────────────────────────────────────────────
bool manualMode  = false;
bool manualPump  = false;
bool pumpState   = false;

BlynkTimer timer;

// ── Blynk Virtual Pins ────────────────────────────────────────────────────────
// V0  Soil Moisture %
// V1  Temperature °C
// V2  Air Humidity %
// V3  Rain status
// V4  Pump status (LED)
// V5  Mode toggle  (0=Auto, 1=Manual)
// V6  



// V5 — Mode toggle
BLYNK_WRITE(V5) {
  manualMode = param.asInt();
  Serial.print("Mode: ");
  Serial.println(manualMode ? "MANUAL" : "AUTO");
}

// V6 — Manual pump control
BLYNK_WRITE(V6) {
  manualPump = param.asInt();
  if (manualMode) {
    setPump(manualPump);
  }
}

// ── Relay helper ─────────────────────────────────────────────────────────────
void setPump(bool on) {
  pumpState = on;
  digitalWrite(RELAY_PUMP_PIN, on ? LOW : HIGH); // active-LOW relay
  Blynk.virtualWrite(V4, on ? 255 : 0);          // LED widget
  Serial.print("Pump: ");
  Serial.println(on ? "ON" : "OFF");
}

// ── Soil moisture read ────────────────────────────────────────────────────────
int readSoilMoisture() {
  int raw = analogRead(SOIL_MOISTURE_PIN);
  int pct = map(raw, SOIL_RAW_DRY, SOIL_RAW_WET, 0, 100);
  return constrain(pct, 0, 100);
}

// ── pH ────────────────────────────────────────────────────────────────────────
float readPH() {
  long sum = 0;
  for (int i = 0; i < PH_SAMPLES; i++) {
    sum += analogRead(PH_PIN);
    delay(10);
  }
  float avgADC  = sum / (float)PH_SAMPLES;
  float voltage = avgADC * (3.3 / 4095.0);
  return PH_SLOPE * voltage + PH_INTERCEPT;
}

// String getPHStatus(float pH) {
//   if      (pH < 5.5) return "Too Acidic!";
//   else if (pH < 6.5) return "Slightly Acidic";
//   else if (pH < 7.5) return "Optimal pH";
//   else if (pH < 8.5) return "Slightly Basic";
//   else               return "Too Alkaline!";
// }

// ── Auto logic ────────────────────────────────────────────────────────────────
void autoLogic(int soilPct, float tempC, bool raining) {
  if (manualMode) return;

  if (raining) {
    if (pumpState) {
      setPump(false);
      Serial.println("Auto: Rain detected — pump OFF");
    }
    return;
  }

  if (soilPct < SOIL_DRY_THRESHOLD) {
    if (!pumpState) setPump(true);
  } else if (soilPct >= SOIL_WET_THRESHOLD) {
    if (pumpState)  setPump(false);
  }

  // Hot day + borderline dry → keep watering a bit longer
  if (tempC > TEMP_HIGH_THRESHOLD && soilPct < (SOIL_DRY_THRESHOLD + 10) && !pumpState) {
    setPump(true);
    Serial.println("Auto: High temp + low moisture — pump ON");
  }
}

// ── Main read + send (every 2 s) ─────────────────────────────────────────────
void readAndSend() {
  int   soilPct  = readSoilMoisture();
  float tempC    = ds18b20.getTempCByIndex(0);
  float humidity = dht.readHumidity();
  bool  raining  = (digitalRead(RAIN_SENSOR_PIN) == LOW);
  int rainRaw = analogRead(RAIN_SENSOR_AO_PIN);
  int rainPct = constrain(map(rainRaw, 4095, 0, 0, 100), 0, 100);

  float  pH     = readPH();
  // String status = getPHStatus(pH);

  ds18b20.requestTemperatures();

  if (isnan(humidity)) humidity = 0;
  if (tempC == DEVICE_DISCONNECTED_C) tempC = 0;

  Blynk.virtualWrite(V0, soilPct);
  Blynk.virtualWrite(V1, tempC);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, raining ? 1 : 0);

  Blynk.virtualWrite(V7, pH);
  Blynk.virtualWrite(V8, rainPct);

  autoLogic(soilPct, tempC, raining);

  Serial.printf("[Soil:%d%%] [Temp:%.1f°C] [Hum:%.1f%%] [Rain:%s | Intensity:%d%%] [Pump:%s | %s]\n",
    soilPct, tempC, humidity,
    raining    ? "YES"    : "NO", rainPct,
   // pH,        phStatus.c_str(),
    manualMode ? "MANUAL" : "AUTO",
    pumpState  ? "ON"    : "OFF"
  );
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PUMP_PIN,  OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PUMP_PIN, HIGH); // start OFF

  dht.begin();
  ds18b20.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(2000L, readAndSend);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
}