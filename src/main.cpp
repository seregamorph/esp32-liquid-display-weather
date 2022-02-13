#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// green
LiquidCrystal_I2C lcd1(0x27, lcdColumns, lcdRows);
// blue
LiquidCrystal_I2C lcd2(0x26, lcdColumns, lcdRows);

void printI2CDevices();

void setup() {
    printI2CDevices();

    lcd1.init();
    lcd1.backlight();

    lcd2.init();
    lcd2.backlight();
}

void loop() {
    lcd1.setCursor(0, 0);
    lcd2.setCursor(0, 0);

    lcd1.print("Hello!");
    lcd2.print("LOL");

    delay(1000);

    lcd1.clear();
    lcd2.clear();

    lcd1.setCursor(0, 1);
    lcd2.setCursor(0, 1);

    lcd1.print("World!");
    lcd2.print("KEK");

    printf("Hello world!\n");

    delay(1000);
    lcd1.clear();
    lcd2.clear();
}

void printI2CDevices() {
    Wire.begin();
    Serial.begin(115200);
    while (!Serial);

    byte error, address;
    int I2CDevices;

    Serial.println("Scanning for I2C Devices...");

    I2CDevices = 0;
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

            I2CDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (I2CDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.println("****\n");
    }
}

/*
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void app_main(void)
{
    printf("Hello world!\n");

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

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds___\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
*/
