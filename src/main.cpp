/*
 * Youth Climate Action Fund Evanston Environmental Monitoring Station (Designed by Martin Suarez and Adan Ciprian)
 *
 * Version: 0.2.0 
 * Version Notes: This bare framework describes preliminary logic for this station's operation, data handling, and architecture.
 * The program has been decently documented, but documentation must be expanded and completed, Further, AI was partially used to assist in
 * the interpretation and understanding of separated components and libraries. Code will be refined and developed by Martin Suarez and Adan Ciprian
 * with sparse AI support (10/30/26).
 *
 * Station Architecture/Logic:
 *  - The station wakes from deep sleep at a specified interval on a timer, reads its sensors, and logs the data to a CSV file on the SD card.
 *  - When you hold the other end of the magnetic contact switch, the station wakes up, starts Bluetooth Low Energy (BLE), and lets your phone/laptop access and download time-sync files, functioning entirely indepent from wifi.
 *  - A fresh power-up or reset also opens a BLE window so that the RTC can be set for the first time.
 *  - Timestamps are recorded in UTC so daylight savings time adjustments are handled correctly.
 * 
 * Hardware:
 * 
 * Controller (Driver):
 *  - Adafruit Feather ESP32 S3 Feather: https://www.adafruit.com/product/5323
 *  - Adafruit Datalogger FeatherWing: https://www.adafruit.com/product/2922
 *  - Adafruit bq25185 USB / DC / Solar Charger with 5V Boost Board: https://www.adafruit.com/product/6106
 *  - Solar Panel (6V 5W): https://www.waveshare.com/solar-panel-6v-5w.htm
 *  - Magnetic Contact Switch: https://www.adafruit.com/product/375
 *
 * Sensors:
 *  - BME680 (I2C): https://www.adafruit.com/product/3660
 *  - GY-906 MLX90614ESF Non-Contact Infrared Temperature Sensor Module IIC I2C Serial for Arduino (I2C): https://www.amazon.com/HiLetgo-MLX90614ESF-Non-contact-Infrared-Temperature/dp/B071VF2RWM
 *  - PM2.5 Air Quality Sensor PMS5003 (UART): https://www.adafruit.com/product/3686
 *
 * Stevenson screen (Case):
 *  - La Crosse Sensor Weather Shield: https://www.lacrossetechnology.com/925-1418-sensor-weather-shield
 *
 * Libraries
 *  - RTClib: https://github.com/adafruit/RTClib
 *  - BME680: https://github.com/adafruit/Adafruit_BME680
 *  - MLX90614: https://github.com/adafruit/Adafruit_MLX90614_Library
 *  - PMS5003: https://learn.adafruit.com/pm25-air-quality-sensor/arduino-code
 *  - BLE, SD, SPI, Wire
 * 
 * Wiring summary (ADAN TO DO):
 *  -PMS5003 TXD -> Feather RX
 *  -PMS5003 RXD -> Feather TX
 *  -PMS5003 SET -> A3
 *  -PMS5003 VCC -> bq251855V
 *  -PMS5003 GND -> GND
 * Data file format: /YYYYMMDD.csv (one file per UTC day)
 * 
 * BLE commands (MARTIN TO DO):
 *  -STATUS
 *  -LIST
 *  -DOWNLOAD
 *  -TIME
 *  -DONE
 * 
 */

/* Dependencies */
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_BME680.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_PM25AQI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <Plantower_PMS5003.h>

/* Settings */
#define STATION_ID "YCAF-01"  // Station number that will show up as BLE name.
#define FIRMWARE_VERSION "0.2.0"  // current firmware version (program/architecture).
#define SAMPLE_INTERVAL_S 600  // 10 minutes between samples.
#define PM_WARMUP_S 30  // PMS5003 warmup time (necessary for stable fan).
#define BLE_REED_WINDOW_S 120  // How long the BLE window is open after contact switch is triggered.
#define BLE_BOOT_WINDOW_S 45  // How long the BLE window is open after power-up/reset.
#define BLE_IDLE_TIMEOUT_S 60  // close the session after this time if no commands received.        
#define BLE_PACE_MS 12  // delay between BLE packets (prevents data from being dropped).
#define DEBUG_SERIAL 1   // Enable or disable USB for field deployment vs debugging.

/* Pin allocations */
#define SD_CS_PIN 10
#define REED_PIN A2   // GPIO16 on the Feather ESP32-S3
#define REED_GPIO GPIO_NUM_16
#define PMS_SET_PIN A3   // GPIO15
#define PMS_SET_GPIO GPIO_NUM_15
#define PMS_RX_PIN RX   // Feather RX  <- PMS5003 TXD
#define PMS_TX_PIN TX   // Feather TX  -> PMS5003 RXD 


/* Debug printing */
#if DEBUG_SERIAL
  #define LOGF(...) Serial.printf(__VA_ARGS__)
#else
  #define LOGF(...)
#endif

/* BLE UUIDs 
 * Using the standard "UART over BLE" UUIDs means free phone apps can talk to the station without any custom software.
 */
#define NUS_SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_RX_UUID   "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // phone -> station (write)
#define NUS_TX_UUID   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // station -> phone (notify)

/* Conserved memory during deep sleep */
RTC_DATA_ATTR uint32_t bootCount = 0;

/* Devices */
RTC_PCF8523   rtc;
Adafruit_BME680   bme;
Adafruit_MLX90614   mlx = Adafruit_MLX90614();
Adafruit_PM25AQI   aqi = Adafruit_PM25AQI();
Adafruit_MAX17048   gauge;

bool bmeOk = false, mlxOk = false, gaugeOk = false, sdOk = false, rtcOk = false;

/* Data in each row for final CSV output */
struct Reading {
  DateTime when;
  float bmeTemp = NAN, bmeHum = NAN, bmePress = NAN, bmeGas = NAN;
  float irAmb = NAN, irObj = NAN;
  float battV = NAN, battPct = NAN;
  int pm1 = -1, pm25 = -1, pm10 = -1;   // -1 means "no reading"
  int p03 = -1, p05 = -1, p10 = -1, p25 = -1, p50 = -1, p100 = -1;
};

const char CSV_HEADER[] =
  "utc,station,bme_temp_c,bme_rh_pct,bme_press_hpa,bme_gas_kohm,"
  "ir_ambient_c,ir_object_c,pm1_0_env,pm2_5_env,pm10_env,"
  "n0_3,n0_5,n1_0,n2_5,n5_0,n10_0,batt_v,batt_pct,boot_count";

/* BLE shared state and parameters */
BLECharacteristic *txChar = nullptr;
volatile bool bleConnected = false;
volatile bool cmdReady = false;
volatile uint16_t peerMtu = 23;
String cmdLine;
bool bleDone = false;

/* Forward function declarations */
bool initRTC();
bool mountSD();
void readTemperatureSensors(Reading &r);
void readParticles(Reading &r);
bool logToSD(const Reading &r);
uint32_t secondsUntilNextSample();
void goToSleep(uint32_t seconds);
void runBleSession(uint32_t windowSeconds);
void handleCommand(String cmd);

/* SETUP - runs on every wake-up (deep sleep restarts the program) */
void setup() {
  // 80 MHz CPU frequency reduces the CPU's power draw.
  setCpuFrequencyMhz(80);

  // Put the PM sensor's SET pin in a known LOW state, THEN release the pin
  // "hold" we set before sleeping (see goToSleep).
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, LOW);
  gpio_hold_dis(PMS_SET_GPIO);
  gpio_deep_sleep_hold_dis();

  // Give the contact switch pin back to normal digital use (it was an RTC pin in sleep).
  rtc_gpio_deinit(REED_GPIO);
  pinMode(REED_PIN, INPUT_PULLDOWN);

#if DEBUG_SERIAL
  Serial.begin(115200);
  delay(500);
#endif

  bootCount++;

  // Determines the reason for waking up from deep sleep.
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bool wokeByMagnet = (cause == ESP_SLEEP_WAKEUP_EXT0);
  bool wokeByTimer = (cause == ESP_SLEEP_WAKEUP_TIMER);
  bool freshBoot  = (!wokeByMagnet && !wokeByTimer);   // power-up, reset, or USB upload
  LOGF("\n=== %s boot #%lu, cause=%d ===\n", STATION_ID, (unsigned long)bootCount, (int)cause);

  Wire.begin();

  rtcOk = initRTC();
  if (!rtcOk) {
    LOGF("ERROR: RTC not found. Check the Datalogger FeatherWing. Retrying next cycle.\n");
    goToSleep(SAMPLE_INTERVAL_S);
  }

  sdOk = mountSD();
  bmeOk = bme.begin();   // potentially need to change to bme.begin(0x76) 
  mlxOk = mlx.begin();
  gaugeOk = gauge.begin();
  LOGF("SD:%d BME680:%d MLX90614:%d Gauge:%d\n", sdOk, bmeOk, mlxOk, gaugeOk);

  if (bmeOk) {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);   // 320 C for 150 ms
  }

  // Bluetooth session (magnet visit, or first power-up)
  if (wokeByMagnet || freshBoot) {
    runBleSession(wokeByMagnet ? BLE_REED_WINDOW_S : BLE_BOOT_WINDOW_S);
  }

  // Measurement (timer wake, or first power-up | will create the first row)
  if (wokeByTimer || freshBoot) {
    Reading r;
    readTemperatureSensors(r);   // first: before the PM sensor's fan/laser warm the enclosure
    readParticles(r);   // ~35 s
    bool saved = logToSD(r);
    LOGF("Logged to SD: %s\n", saved ? "yes" : "NO (SD problem)");
  }

  goToSleep(secondsUntilNextSample());
}

void loop() {}   // necessary because of the Arduino framework requirement (never reached)

/* Clock + SD */
bool initRTC() {
  if (!rtc.begin()) return false;
  if (!rtc.initialized() || rtc.lostPower()) {
    LOGF("RTC lost power: set a placeholder time. Send TIME over BLE!\n");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   // placeholder
  }
  rtc.start();
  return true;
}

bool mountSD() {
  if (!SD.begin(SD_CS_PIN)) return false;
  return SD.cardType() != CARD_NONE;
}

/* Sensors */
void readTemperatureSensors(Reading &r) {
  r.when = rtc.now();   // timestamp = moment of this reading

  if (bmeOk && bme.performReading()) {
    r.bmeTemp = bme.temperature;
    r.bmeHum = bme.humidity;
    r.bmePress = bme.pressure / 100.0;   // Pa -> hPa
    r.bmeGas = bme.gas_resistance / 1000.0;   // ohm -> kOhm
  } else {
    LOGF("BME680 reading failed\n");
  }

  if (mlxOk) {
    r.irAmb = mlx.readAmbientTempC();
    r.irObj = mlx.readObjectTempC();
  }

  if (gaugeOk) {
    r.battV = gauge.cellVoltage();
    r.battPct = gauge.cellPercent();
  }
}

void readParticles(Reading &r) {
  Serial1.begin(9600, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);
  aqi.begin_UART(&Serial1);

  digitalWrite(PMS_SET_PIN, HIGH);   // wake fan + laser
  delay(PM_WARMUP_S * 1000UL);

  // The UART buffer holds old frames from the warm-up. This ensures we only process fresh data.
  while (Serial1.available()) Serial1.read();

  PM25_AQI_Data d;
  bool got = false;
  uint32_t t0 = millis();
  while (!got && millis() - t0 < 5000) {
    got = aqi.read(&d);
    if (!got) delay(100);
  }

  digitalWrite(PMS_SET_PIN, LOW);   // back to standby
  Serial1.end();

  if (!got) { LOGF("PMS5003: no valid frame\n"); return; }

  r.pm1  = d.pm10_env;   // "pm10_env" is PM1.0 weird notation lol
  r.pm25 = d.pm25_env;
  r.pm10 = d.pm100_env;   // "pm100_env" = PM10
  r.p03  = d.particles_03um; // not sure if we have get all below data — will check documentation.
  r.p05  = d.particles_05um;
  r.p10  = d.particles_10um;
  r.p25  = d.particles_25um;
  r.p50  = d.particles_50um;
  r.p100 = d.particles_100um;
}

/* Logging */
bool logToSD(const Reading &r) {
  if (!sdOk) return false;

  char path[16];
  snprintf(path, sizeof(path), "/%04d%02d%02d.csv", r.when.year(), r.when.month(), r.when.day());

  bool isNew = !SD.exists(path);
  File f = SD.open(path, FILE_APPEND);
  if (!f) return false;

  if (isNew) { f.print(CSV_HEADER); f.print("\n"); }

  char line[256];
  snprintf(line, sizeof(line),
    "%04d-%02d-%02dT%02d:%02d:%02dZ,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,"
    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%.3f,%.1f,%lu\n",
    r.when.year(), r.when.month(), r.when.day(),
    r.when.hour(), r.when.minute(), r.when.second(),
    STATION_ID,
    r.bmeTemp, r.bmeHum, r.bmePress, r.bmeGas,
    r.irAmb, r.irObj,
    r.pm1, r.pm25, r.pm10,
    r.p03, r.p05, r.p10, r.p25, r.p50, r.p100,
    r.battV, r.battPct,
    (unsigned long)bootCount);

  f.print(line);
  f.close();   // close = send data to card 
  LOGF("%s", line);
  return true;
}

/* Sleep */

// Seconds from now until a few seconds before the next data collection time
// All stations sample at the same clock times for easy comparisons.
uint32_t secondsUntilNextSample() {
  if (!rtcOk) return SAMPLE_INTERVAL_S;
  const uint32_t lead = 3;   // wake early for boot + sensor init
  uint32_t now  = rtc.now().unixtime();
  uint32_t slot = (now / SAMPLE_INTERVAL_S + 1) * SAMPLE_INTERVAL_S;
  if (slot - now <= lead + 1) slot += SAMPLE_INTERVAL_S;
  return slot - lead - now;
}

void goToSleep(uint32_t seconds) {
  LOGF("Deep sleep for %lu s\n", (unsigned long)seconds);

  // AI said this (worth checking it's factualness): 1) Keep the PM sensor in standby while the MCU sleeps. Without "hold", GPIO pins float in deep sleep and the sensor would wake itself.
  digitalWrite(PMS_SET_PIN, LOW);
  gpio_hold_en(PMS_SET_GPIO);
  gpio_deep_sleep_hold_en();

  // Also AI FACT CHECK! (contact switch and REED switch are interchangeable) 2) If the magnet is still on the reed switch, wait (up to 30 s) for it to be removed; otherwise we would wake again instantly.
  uint32_t t0 = millis();
  while (digitalRead(REED_PIN) == HIGH && millis() - t0 < 30000) delay(100);
  bool magnetStillThere = (digitalRead(REED_PIN) == HIGH);

  // Wake sources: timer always, contact switch only if it is not currently in place /active.
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  if (!magnetStillThere) {
    rtc_gpio_init(REED_GPIO);
    rtc_gpio_set_direction(REED_GPIO, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis(REED_GPIO);
    rtc_gpio_pulldown_en(REED_GPIO);
    esp_sleep_enable_ext0_wakeup(REED_GPIO, 1);   // wake when pin goes HIGH
  }

#if DEBUG_SERIAL
  Serial.flush();
#endif
  esp_deep_sleep_start();
}

/* BLE */
class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override    { bleConnected = true; peerMtu = 23; }
  void onDisconnect(BLEServer *s) override { bleConnected = false; }
  void onMtuChanged(BLEServer *s, esp_ble_gatts_cb_param_t *p) override { peerMtu = p->mtu.mtu; }
};

class RxCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String v = c->getValue().c_str();   // works on core 2.x and 3.x
    v.trim();
    if (v.length() == 0) return;
    cmdLine = v;
    cmdReady = true;
  }
};

size_t bleChunkSize() {
  size_t c = (peerMtu > 23) ? (peerMtu - 3) : 20;
  return c > 240 ? 240 : c;
}

void bleSendRaw(const uint8_t *data, size_t len) {  // len must be <= bleChunkSize() due to data limitations
  if (!bleConnected || len == 0) return;
  txChar->setValue((uint8_t *)data, len);
  txChar->notify();
  delay(BLE_PACE_MS);
}

void bleSendLine(const String &s) {
  String l = s + "\n";
  size_t chunk = bleChunkSize(), off = 0;
  while (off < l.length() && bleConnected) {
    size_t n = min(chunk, (size_t)(l.length() - off));
    bleSendRaw((const uint8_t *)l.c_str() + off, n);
    off += n;
  }
}

// Validate file names to ensure they follow the expected format (8 digits + ".csv").
bool validName(const String &n) {
  if (n.length() != 12 || !n.endsWith(".csv")) return false;
  for (int i = 0; i < 8; i++) if (!isDigit(n[i])) return false;
  return true;
}

void sendList() {
  if (!sdOk) { bleSendLine("ERR sd"); return; }
  File root = SD.open("/");
  File f = root.openNextFile();
  while (f) {
    String n = f.name();
    if (n.startsWith("/")) n.remove(0, 1);
    if (!f.isDirectory() && validName(n)) {
      bleSendLine("FILE " + n + " " + String((unsigned long)f.size()));
    }
    f.close();
    f = root.openNextFile();
  }
  root.close();
  bleSendLine("END");
}

void sendFile(const String &name, uint32_t offset) {
  if (!sdOk) { bleSendLine("ERR sd"); return; }
  if (!validName(name)) { bleSendLine("ERR badname"); return; }
  File f = SD.open("/" + name, FILE_READ);
  if (!f) { bleSendLine("ERR nofile"); return; }
  if (offset > f.size()) offset = 0;
  f.seek(offset);

  uint8_t buf[240];
  uint32_t sent = 0;
  size_t chunk = bleChunkSize();
  size_t n;
  while (bleConnected && (n = f.read(buf, chunk)) > 0) {
    bleSendRaw(buf, n);
    sent += n;
  }
  f.close();
  bleSendLine("EOF " + name + " " + String((unsigned long)(offset + sent)));
}

void handleCommand(String cmd) {
  LOGF("BLE command: %s\n", cmd.c_str());

  if (cmd == "LIST") {
    sendList();

  } else if (cmd.startsWith("GET ")) {
    String rest = cmd.substring(4);
    rest.trim();
    int sp = rest.indexOf(' ');
    String name = (sp < 0) ? rest : rest.substring(0, sp);
    uint32_t off = (sp < 0) ? 0 : (uint32_t)rest.substring(sp + 1).toInt();
    sendFile(name, off);

  } else if (cmd.startsWith("TIME ")) {
    uint32_t t = (uint32_t)strtoul(cmd.substring(5).c_str(), nullptr, 10);
    if (t > 1700000000UL) {                             // sanity check: after Nov 2023
      rtc.adjust(DateTime(t));
      rtc.start();
      bleSendLine("OK TIME " + String((unsigned long)t));
    } else {
      bleSendLine("ERR time");
    }

  } else if (cmd == "STATUS") {
    char buf[200];
    DateTime n = rtc.now();
    snprintf(buf, sizeof(buf),
      "STATUS id=%s fw=%s utc=%04d-%02d-%02dT%02d:%02d:%02dZ batt=%.2fV sd=%s bme=%d mlx=%d boots=%lu",
      STATION_ID, FIRMWARE_VERSION,
      n.year(), n.month(), n.day(), n.hour(), n.minute(), n.second(),
      gaugeOk ? gauge.cellVoltage() : NAN,
      sdOk ? "ok" : "FAIL", bmeOk, mlxOk, (unsigned long)bootCount);
    bleSendLine(String(buf));

  } else if (cmd == "DONE") {
    bleSendLine("OK bye");
    bleDone = true;

  } else {
    bleSendLine("ERR commands: STATUS | LIST | GET <file> [offset] | TIME <unix> | DONE");
  }
}

void runBleSession(uint32_t windowSeconds) {
  LOGF("BLE on, waiting up to %lu s\n", (unsigned long)windowSeconds);
  bleDone = false;
  cmdReady = false;

  BLEDevice::init(STATION_ID);
  BLEDevice::setMTU(247);   // ask for big packets (faster transfer)

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCB());

  BLEService *svc = server->createService(NUS_SERVICE_UUID);

  txChar = svc->createCharacteristic(NUS_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  txChar->addDescriptor(new BLE2902());

  BLECharacteristic *rxChar = svc->createCharacteristic(
      NUS_RX_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxChar->setCallbacks(new RxCB());

  svc->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  uint32_t deadline = millis() + windowSeconds * 1000UL;
  bool wasConnected = false;

  while (millis() < deadline && !bleDone) {
    if (bleConnected && !wasConnected) {                // someone just connected
      deadline = millis() + BLE_IDLE_TIMEOUT_S * 1000UL;
      wasConnected = true;
    }
    if (!bleConnected && wasConnected) {                // link dropped: allow a reconnect
      BLEDevice::startAdvertising();
      wasConnected = false;
    }
    if (cmdReady) {
      cmdReady = false;
      String c = cmdLine;
      handleCommand(c);
      deadline = millis() + BLE_IDLE_TIMEOUT_S * 1000UL;   // activity resets the idle timer
    }
    delay(20);
  }

  delay(300);                                           // let the last packets leave
  BLEDevice::deinit(true);                              // radio off
  bleConnected = false;
  LOGF("BLE session closed\n");
}
