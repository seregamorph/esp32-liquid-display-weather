#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <stdio.h>

#include <string>

#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/task.h"
#include "sdkconfig.h"
// wifi creds here
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "HardwareSerial.h"
#include "TinyGPSPlus.h"
#include "secret.h"
#include "types.h"

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// green
LiquidCrystal_I2C lcd1(0x27, lcdColumns, lcdRows);
// blue
LiquidCrystal_I2C lcd2(0x26, lcdColumns, lcdRows);

TinyGPSPlus gps;
HardwareSerial SerialGPS(2);

void printI2CDevices();

void print_chip_info();

void print_wifi_info();

void setup() {
    Serial.begin(115200);
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

    printI2CDevices();
    print_chip_info();
    // print_wifi_info();

    lcd1.init();
    lcd1.backlight();

    lcd2.init();
    lcd2.backlight();

    lcd1.clear();
    lcd2.clear();
}

void loop() {
    lcd1.setCursor(0, 0);
    lcd2.setCursor(0, 0);

    lcd1.print("GPS connecting");

    Serial.println("Connecting GPS");

    while (gps.satellites.value() < 4) {
        while (SerialGPS.available() > 0) {
            char ch = SerialGPS.read();
            Serial.write(ch);
            gps.encode(ch);
        }

        Serial.println();
        Serial.print("LAT=");
        Serial.println(gps.location.lat(), 6);
        Serial.print("LONG=");
        Serial.println(gps.location.lng(), 6);
        Serial.print("ALT=");
        Serial.println(gps.altitude.meters());
        Serial.print("Time=");
        Serial.println(gps.time.value());
        Serial.print("Satellites=");
        Serial.println(gps.satellites.value());

        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.printf("Satellittes: %d", gps.satellites.value());
        delay(1000);
    }

    Serial.println();

    Serial.print("Connecting to WiFi ..");
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Connecting to WiFi");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        // observed sequence of Wifi statuses: 6 (disconnected), 0 (idle), 3 (connected)
        // or constant 6 if WiFi password is wrong
        // or 1 if the network SSID is not correct
        Serial.print('.');

        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Connecting");

        lcd2.setCursor(0, 1);
        lcd2.printf("%d", WiFi.status());

        delay(500);
    }
    Serial.println(WiFi.localIP());

    //    delay(1000);

    lcd1.clear();
    lcd2.clear();

    lcd1.setCursor(0, 1);
    lcd2.setCursor(0, 1);

    lcd1.print("World!");

    lcd2.setCursor(0, 0);
    lcd2.print("Connected");
    lcd2.setCursor(0, 1);
    lcd2.print(WiFi.localIP());

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Requesting weather");

    HTTPClient http;
    String weatherUrl = String("http://api.openweathermap.org/data/2.5/weather?lat=") +
                        String(gps.location.lat(), 4) +
                        String("&lon=") +
                        String(gps.location.lng(), 4) +
                        String("&appid=") +
                        OPENWEATHERMAP_TOKEN;
    Serial.println(weatherUrl);
    http.begin(weatherUrl.c_str());
    int httpResponseCode = http.GET();

    lcd1.clear();
    lcd2.clear();

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);

        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload.c_str());

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            delay(600000);
        }

        lcd1.setCursor(0, 0);
        String name = doc["name"];
        lcd1.print(name);

        lcd2.setCursor(0, 0);
        lcd2.print("Temp");

        lcd2.setCursor(0, 1);
        const double temp_k = doc["main"]["temp"];
        const double temp_c = temp_k - 273.15;
        lcd2.print(temp_c);
        lcd2.print(" degree");
        // lcd2.printf("", );
        //        lcd2.setCursor(0, 1);
        //        lcd2.print(payload.length());
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);

        lcd2.setCursor(0, 0);
        lcd2.printf("Error %d", httpResponseCode);
    }
    // Free resources
    http.end();

    delay(600000);
    lcd1.clear();
    lcd2.clear();
}

void printI2CDevices() {
    Wire.begin();

    byte error, address;
    int i2CDevices;

    Serial.println("Scanning for I2C Devices...");

    i2CDevices = 0;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println(" !");

            i2CDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (i2CDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.println("****\n");
    }
}

void print_chip_info() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    fflush(stdout);
}

/*
void print_wifi_info() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    Serial.println("WiFi setup...");

    WiFi.mode(WIFI_STA);
    Serial.println("WiFi mode...");
    WiFi.disconnect();
    Serial.println("WiFi disconnect...");
    delay(100);

    Serial.println("WiFi setup done");

    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");
}
*/
