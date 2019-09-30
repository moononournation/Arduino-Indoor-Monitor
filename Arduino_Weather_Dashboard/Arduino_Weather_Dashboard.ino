#include <SPI.h>
#include <Arduino_HWSPI.h>
#include <Arduino_GFX.h>    // Core graphics library derived from Adafruit_GFX
#include <Arduino_ST7735.h> // Hardware-specific library for ST7735 (with or without CS pin)
#include "FreeMonoBold9pt7b.h"
#include <DHTesp.h>
#include <Ticker.h>

#define TFT_BL 22

Arduino_HWSPI *bus = new Arduino_HWSPI(16 /* DC */, 5 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */);
Arduino_ST7735 *tft = new Arduino_ST7735(bus, 17 /* RST */, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);

DHTesp dht;
uint16_t panel_color_1;
uint16_t panel_color_2;
uint16_t panel_color_3;
uint16_t panel_color_4;
uint16_t panel_color_5;
uint16_t panel_color_6;

void indoorDataTask(void *pvParameters);
bool updateIndoorData();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t indoorDataTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int airPin = 4;
int dhtPin = 21;

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp()
{
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      indoorDataTask,        /* Function to implement the task */
      "indoorDataTask ",     /* Name of the task */
      4000,                  /* Stack size in words */
      NULL,                  /* Task input parameter */
      5,                     /* Priority of the task */
      &indoorDataTaskHandle, /* Task handle. */
      1);                    /* Core where the task should run */

  if (indoorDataTaskHandle == NULL)
  {
    Serial.println("Failed to start task for temperature update");
    return false;
  }
  else
  {
    // Start update of environment data every 20 seconds
    tempTicker.attach(4, triggerGetTemp);
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp()
{
  if (indoorDataTaskHandle != NULL)
  {
    xTaskResumeFromISR(indoorDataTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void indoorDataTask(void *pvParameters)
{
  Serial.println("indoorDataTask loop started");
  while (1) // indoorDataTask loop
  {
    if (tasksEnabled)
    {
      // Get temperature values
      updateIndoorData();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

/**
 * updateIndoorData
 * Reads indoor air quality from MQ139 sensor
 * and reads temperature and humidity from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool updateIndoorData()
{
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0)
  {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return false;
  }

  int air_quality = analogRead(airPin) * 100.0 / 4096.0;
  int temperature = newValues.temperature;
  int humidity = newValues.humidity;

  // for Serial Plotter
  Serial.println(String(air_quality) + " " + String(newValues.temperature) + " " + String(newValues.humidity));

  // print indoor data
  tft->setFont(&FreeMonoBold9pt7b);

  tft->setCursor(8, 81);
  tft->setTextColor(BLACK, panel_color_1);
  if (air_quality < 10)
  {
    tft->print(" ");
  }
  tft->println(air_quality);

  tft->setCursor(49, 81);
  tft->setTextColor(BLACK, panel_color_2);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->println(temperature);

  tft->setCursor(92, 81);
  tft->setTextColor(BLACK, panel_color_3);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->println(humidity);

  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("DHT ESP32 example with tasks");

  tft->begin();
  tft->fillScreen(BLACK);
  tft->setTextColor(RED);

  tft->setFont(&FreeMonoBold9pt7b);
  tft->setCursor(0, 10);
  tft->println("Weather");
  tft->setTextColor(YELLOW);
  tft->println("  Dashboard");

  panel_color_1 = tft->color565(248,  64,  64);
  panel_color_2 = tft->color565(248, 128,  16);
  panel_color_3 = tft->color565(240, 240,  16);
  panel_color_4 = tft->color565(128, 248,  16);
  panel_color_5 = tft->color565( 16, 248, 128);
  panel_color_6 = tft->color565( 64,  64, 248);

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

  tft->fillRoundRect(0, 50, 40, 40, 5, panel_color_1);
  tft->fillRoundRect(44, 50, 40, 40, 5, panel_color_2);
  tft->fillRoundRect(88, 50, 40, 40, 5, panel_color_3);

  tft->fillRoundRect(0, 110, 40, 40, 5, panel_color_4);
  tft->fillRoundRect(44, 110, 40, 40, 5, panel_color_5);
  tft->fillRoundRect(88, 110, 40, 40, 5, panel_color_6);

  tft->drawCircle(74, 71, 2, BLACK);

  tft->setCursor(114, 81);
  tft->setTextColor(BLACK, panel_color_3);
  tft->print("%");

  tft->setFont(0);
  tft->setTextColor(WHITE);
  tft->setCursor(0, 40);
  tft->println("Indoor");
  tft->setCursor(0, 100);
  tft->println("HK Observatory");
  tft->setCursor(0, 152);
  tft->println("Last Update:");
  tft->setTextColor(BLACK);
  tft->setCursor(5, 55);
  tft->println("Air Q");
  tft->setCursor(52, 55);
  tft->println("Temp");
  tft->setCursor(96, 55);
  tft->println("Humi");

  pinMode(airPin, INPUT);
  initTemp();

  // Signal end of setup() to tasks
  tasksEnabled = true;
}

void loop()
{
  if (!tasksEnabled)
  {
    // Wait 2 seconds to let system settle down
    delay(2000);
    // Enable task that will read values from the DHT sensor
    tasksEnabled = true;
    if (indoorDataTaskHandle != NULL)
    {
      vTaskResume(indoorDataTaskHandle);
    }
  }
  yield();
}
