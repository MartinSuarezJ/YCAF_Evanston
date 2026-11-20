// Code to test and prototype with the Adalogger FeatherWing (PCF8523 RTC + microSD)

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

// Wiring: the FeatherWing stacks directly on the Feather ESP32-S3
//   RTC  -> I2C (SDA/SCL), address 0x68
//   SD   -> SPI (SCK/MOSI/MISO), CS on pin 10 (same as src/main.cpp)
#define SD_CS_PIN 10

#define LOG_PATH        "/dl_test.csv"
#define TEST_PATH       "/rw_test.txt"
#define LOG_INTERVAL_MS 5000

RTC_PCF8523 rtc;
bool rtcOk = false;
bool sdOk  = false;
uint32_t logCount = 0;

// "YYYY-MM-DD HH:MM:SS"
String timestamp(const DateTime &t) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
    return String(buf);
}

// ---------------- RTC tests ----------------
bool testRTC() {
    Serial.println(F("\n[RTC] Starting..."));
    if (!rtc.begin()) {
        Serial.println(F("[RTC] FAIL: not found on I2C. Is the FeatherWing seated?"));
        return false;
    }
    Serial.println(F("[RTC] Found PCF8523"));

    if (!rtc.initialized() || rtc.lostPower()) {
        Serial.println(F("[RTC] Lost power / never set -> setting to compile time"));
        Serial.println(F("[RTC] (check the CR1220 coin cell if this happens every boot)"));
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    rtc.start();

    // Check that the clock is actually ticking
    DateTime a = rtc.now();
    Serial.print(F("[RTC] Time now: ")); Serial.println(timestamp(a));
    delay(2100);
    DateTime b = rtc.now();
    int32_t diff = (int32_t)(b.unixtime() - a.unixtime());
    Serial.printf("[RTC] Elapsed over ~2.1 s: %ld s\n", (long)diff);
    if (diff < 1 || diff > 3) {
        Serial.println(F("[RTC] FAIL: clock is not ticking correctly"));
        return false;
    }

    // Sanity check on the year (catches garbage reads)
    if (b.year() < 2024 || b.year() > 2099) {
        Serial.println(F("[RTC] WARN: year looks wrong - time may need setting"));
    }
    Serial.println(F("[RTC] PASS"));
    return true;
}

// ---------------- SD tests ----------------
bool testSD() {
    Serial.println(F("\n[SD] Starting..."));
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println(F("[SD] FAIL: SD.begin() failed. Card inserted? FAT32 formatted?"));
        return false;
    }

    uint8_t type = SD.cardType();
    if (type == CARD_NONE) {
        Serial.println(F("[SD] FAIL: no card detected"));
        return false;
    }
    const char *typeName = (type == CARD_MMC)  ? "MMC"  :
                           (type == CARD_SD)   ? "SDSC" :
                           (type == CARD_SDHC) ? "SDHC" : "UNKNOWN";
    Serial.printf("[SD] Card type: %s\n", typeName);
    Serial.printf("[SD] Card size: %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
    Serial.printf("[SD] Used: %llu KB / %llu KB\n",
                  SD.usedBytes() / 1024ULL, SD.totalBytes() / 1024ULL);

    // Write -> read back -> compare
    const char *msg = "Hello from the YCAF datalogger test!";
    if (SD.exists(TEST_PATH)) SD.remove(TEST_PATH);

    File f = SD.open(TEST_PATH, FILE_WRITE);
    if (!f) { Serial.println(F("[SD] FAIL: could not open file for write")); return false; }
    f.print(msg);
    f.close();

    f = SD.open(TEST_PATH, FILE_READ);
    if (!f) { Serial.println(F("[SD] FAIL: could not open file for read")); return false; }
    String readBack = f.readString();
    f.close();

    if (readBack != msg) {
        Serial.println(F("[SD] FAIL: read-back mismatch"));
        Serial.print(F("[SD]   wrote: ")); Serial.println(msg);
        Serial.print(F("[SD]   read : ")); Serial.println(readBack);
        return false;
    }
    Serial.println(F("[SD] Write/read-back OK"));

    // Append test
    f = SD.open(TEST_PATH, FILE_APPEND);
    if (!f) { Serial.println(F("[SD] FAIL: could not open file for append")); return false; }
    f.print("+more");
    f.close();
    f = SD.open(TEST_PATH, FILE_READ);
    size_t sz = f.size();
    f.close();
    if (sz != strlen(msg) + 5) {
        Serial.printf("[SD] FAIL: append size %u, expected %u\n", (unsigned)sz, (unsigned)(strlen(msg) + 5));
        return false;
    }
    Serial.println(F("[SD] Append OK"));

    SD.remove(TEST_PATH);
    Serial.println(F("[SD] PASS"));
    return true;
}

void listRoot() {
    Serial.println(F("\n[SD] Files in /:"));
    File root = SD.open("/");
    File entry = root.openNextFile();
    while (entry) {
        Serial.printf("  %-24s %8u bytes%s\n", entry.name(), (unsigned)entry.size(),
                      entry.isDirectory() ? " <DIR>" : "");
        entry.close();
        entry = root.openNextFile();
    }
    root.close();
}

// Serial commands:
//   T<unix seconds>  set RTC (e.g. on a PC: date +%s)
//   D                dump the log file
//   X                delete the log file
//   L                list files
void handleSerial() {
    if (!Serial.available()) return;
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    char c = toupper(cmd[0]);
    if (c == 'T' && rtcOk) {
        uint32_t t = cmd.substring(1).toInt();
        if (t > 1700000000UL) {
            rtc.adjust(DateTime(t));
            rtc.start();
            Serial.print(F("RTC set to ")); Serial.println(timestamp(rtc.now()));
        } else {
            Serial.println(F("Bad time. Use T<unix seconds>"));
        }
    } else if (c == 'D' && sdOk) {
        File f = SD.open(LOG_PATH, FILE_READ);
        if (!f) { Serial.println(F("No log file")); return; }
        Serial.println(F("----- " LOG_PATH " -----"));
        while (f.available()) Serial.write(f.read());
        f.close();
        Serial.println(F("----- end -----"));
    } else if (c == 'X' && sdOk) {
        Serial.println(SD.remove(LOG_PATH) ? F("Log deleted") : F("Delete failed"));
    } else if (c == 'L' && sdOk) {
        listRoot();
    } else {
        Serial.println(F("Commands: T<unix>, D (dump), X (delete log), L (list)"));
    }
}

void setup() {
    Serial.begin(115200);
    // USB CDC: wait (up to 5 s) for the monitor so the first prints aren't lost
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    Serial.println(F("Adalogger FeatherWing test (PCF8523 RTC + microSD)"));

    Wire.begin();
    rtcOk = testRTC();
    sdOk  = testSD();
    if (sdOk) listRoot();

    Serial.println(F("\n========== SUMMARY =========="));
    Serial.printf("RTC: %s\n", rtcOk ? "PASS" : "FAIL");
    Serial.printf("SD : %s\n", sdOk  ? "PASS" : "FAIL");
    Serial.println(F("============================="));
    Serial.println(F("Commands: T<unix>, D (dump), X (delete log), L (list)\n"));

    if (sdOk && !SD.exists(LOG_PATH)) {
        File f = SD.open(LOG_PATH, FILE_WRITE);
        if (f) { f.println("timestamp,unix,millis,count"); f.close(); }
    }
}

void loop() {
    handleSerial();

    static uint32_t lastLog = 0;
    if (millis() - lastLog < LOG_INTERVAL_MS) return;
    lastLog = millis();

    DateTime now = rtcOk ? rtc.now() : DateTime((uint32_t)0);
    String line = timestamp(now) + "," + String(now.unixtime()) + "," +
                  String(millis()) + "," + String(++logCount);

    bool written = false;
    if (sdOk) {
        File f = SD.open(LOG_PATH, FILE_APPEND);
        if (f) {
            written = f.println(line) > 0;
            f.close();
        }
    }
    Serial.printf("%s  -> %s\n", line.c_str(), written ? "logged" : "NOT LOGGED");
}
