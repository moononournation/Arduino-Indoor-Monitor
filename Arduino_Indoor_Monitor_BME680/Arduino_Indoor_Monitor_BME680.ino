#include <Arduino_GFX_Library.h>
#define TFT_BL 4
Arduino_DataBus *bus = new Arduino_ESP32SPI(16 /* DC */, 5 /* CS */, 18 /* SCK */, 19 /* MOSI */, -1 /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 23 /* RST */, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */, 52 /* col offset 1 */, 40 /* row offset 1 */, 53 /* col offset 2 */, 40 /* row offset 2 */);

#include "Zanshin_BME680.h"  // Include the BME680 Sensor library
BME680_Class BME680;  ///< Create an instance of the BME680 class

void setup(void)
{
    Serial.begin(115200);

    gfx->begin();
    gfx->fillScreen(BLACK);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    gfx->setTextColor(RED);
    gfx->setTextSize(4 /* x scale */, 4 /* y scale */, 1 /* pixel_margin */);
    gfx->setCursor(0, 0);
    gfx->println('B');
    gfx->setCursor(22, 0);
    gfx->println('M');
    gfx->setCursor(44, 0);
    gfx->println('E');
    gfx->setCursor(66, 0);
    gfx->println('6');
    gfx->setCursor(88, 0);
    gfx->println('8');
    gfx->setCursor(110, 0);
    gfx->println('0');

    Serial.print(F("Starting I2CDemo example program for BME680\n"));
    Serial.print(F("- Initializing BME680 sensor\n"));
    while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
        Serial.print(F("-  Unable to find BME680. Trying again in 5 seconds.\n"));
        delay(5000);
    }  // of loop until device is located
    Serial.print(F("- Setting 16x oversampling for all sensors\n"));
    BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
    BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
    BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
    Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
    BME680.setIIRFilter(IIR4);  // Use enumerated type values
    Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));
    BME680.setGas(320, 150);  // 320�c for 150 milliseconds
}

void loop()
{
    static int32_t  temp, humidity, pressure, gas;  // BME readings
    static char     buf[16];                        // sprintf text buffer
    static uint16_t loopCounter = 0;                // Display iterations
    if (loopCounter % 25 == 0) {                    // Show header @25 loops
      Serial.print(F("\nLoop Temp\xC2\xB0\x43 Humid% Press hPa Air m\xE2\x84\xA6\n"));
      Serial.print(F("==== ====== ====== ========= ======\n"));
    }                                                     // if-then time to show headers
    BME680.getSensorData(temp, humidity, pressure, gas);  // Get readings
    if (loopCounter++ != 0) {                             // Ignore first reading, might be incorrect
      sprintf(buf, "%4d %3d.%02d", (loopCounter - 1) % 9999,  // Clamp to 9999,
              (int8_t)(temp / 100), (uint8_t)(temp % 100));   // Temp in decidegrees
      Serial.print(buf);
      gfx->setCursor(9, 60);
      gfx->setTextColor(ORANGE, BLACK);
      gfx->setTextSize(5 /* x scale */, 5 /* y scale */, 1 /* pixel_margin */);
      gfx->print(temp / 100);
      gfx->print("\xF8""C");

      sprintf(buf, "%3d.%03d", (int8_t)(humidity / 1000),
              (uint16_t)(humidity % 1000));  // Humidity milli-pct
      Serial.print(buf);
      gfx->setCursor(24, 120);
      gfx->setTextColor(BLUE, BLACK);
      gfx->setTextSize(5 /* x scale */, 5 /* y scale */, 1 /* pixel_margin */);
      gfx->print(humidity / 1000);
      gfx->print('%');

      sprintf(buf, "%7d.%02d", (int16_t)(pressure / 100),
              (uint8_t)(pressure % 100));  // Pressure Pascals
      Serial.print(buf);
      gfx->setCursor(0, 190);
      gfx->setTextColor(CYAN, BLACK);
      gfx->setTextSize(2);
      gfx->print(0.01 * pressure);
      gfx->print(" hPa");

      sprintf(buf, "%4d.%02d\n", (int16_t)(gas / 100), (uint8_t)(gas % 100));  // Resistance milliohms
      Serial.print(buf);
      gfx->setCursor(0, 220);
      gfx->setTextColor(GREEN, BLACK);
      gfx->setTextSize(2);
      gfx->print("IAQ: ");
      gfx->print(0.001 * gas);

      delay(10000);  // Wait 10s
    }                // of ignore first reading
}
