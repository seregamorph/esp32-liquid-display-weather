#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include <stdio.h>
#include <WiFi.h>
#include "sdkconfig.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
// wifi creds here
#include "secret.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
String weatherUrl = "http://api.weatherapi.com/v1/current.json?key=" + WEATHER_API_TOKEN + "&q=Amstelveen&aqi=no";

// green
LiquidCrystal_I2C lcd1(0x27, lcdColumns, lcdRows);
// blue
LiquidCrystal_I2C lcd2(0x26, lcdColumns, lcdRows);

void printI2CDevices();

void print_chip_info();

void print_wifi_info();

void setup() {
    Serial.begin(115200);

    printI2CDevices();
    print_chip_info();
    print_wifi_info();

    lcd1.init();
    lcd1.backlight();

    lcd2.init();
    lcd2.backlight();
}

void loop() {
    lcd1.setCursor(0, 0);
    lcd2.setCursor(0, 0);

    lcd1.print("Hello!");

    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi ..");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        // observer sequence of Wifi statuses: 6 (disconnected), 0 (idle), 3 (connected)
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

    printf("loop\n");

    HTTPClient http;
    http.begin(weatherUrl.c_str());
    int httpResponseCode = http.GET();

    lcd1.clear();
    lcd2.clear();

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);

        lcd1.setCursor(0, 0);
        lcd1.print("Amstelveen");

        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload.c_str());

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            delay(600000);
        }

        //JSONVar myObject = JSON.parse(sensorReadings);

        lcd2.setCursor(0, 0);
        lcd2.print("Temp");

        lcd2.setCursor(0, 1);
        const double temp_c = doc["current"]["temp_c"];
        lcd2.print(temp_c);
        //lcd2.printf("", );
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
