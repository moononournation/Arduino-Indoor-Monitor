/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
#define NTP_SERVER "stdtime.gov.hk"
#define GMT_OFFSET_SEC 28800L  // Timezone +0800
#define DAYLIGHT_OFFSET_SEC 0L // no daylight saving
#define UPDATE_INTERVAL 4      // sensors update interval in seconds
#define WEATHER_RSS_URL "http://rss.weather.gov.hk/rss/CurrentWeather.xml"

/* display settings */
#include <SPI.h>
#include <Arduino_ESP32SPI.h>
#include <Arduino_GFX.h>    // Core graphics library derived from Adafruit_GFX
#include <Arduino_ST7735.h> // Hardware-specific library for ST7735 (with or without CS pin)

Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(16 /* DC */, 5 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */);
Arduino_ST7735 *tft = new Arduino_ST7735(bus, 17 /* RST */, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);
#define TFT_BL 22
#define AIR_PIN 34
#define DHT_PIN 21

#include <esp_task_wdt.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <time.h>
#include <WiFi.h>

#include <DHTesp.h>
#include "FreeMonoBold9pt7b.h"

static struct tm timeinfo;
static int8_t last_rss_update_hour = -2; // never updated
static HTTPClient http;
static int len;

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
  dht.setup(DHT_PIN, DHTesp::DHT11);
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
    // Start update of environment data every UPDATE_INTERVAL
    tempTicker.attach(UPDATE_INTERVAL, triggerGetTemp);
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

  float air_quality = analogRead(AIR_PIN) * 10.0 / 4096.0;
  int temperature = newValues.temperature;
  int humidity = newValues.humidity;

  // for Serial Plotter
  Serial.println(String(air_quality) + " " + String(newValues.temperature) + " " + String(newValues.humidity));

  // print indoor data
  tft->setFont(&FreeMonoBold9pt7b);

  tft->setCursor(3, 81);
  tft->setTextColor(BLACK, panel_color_1);
  tft->println(air_quality, 1);

  tft->setCursor(50, 81);
  tft->setTextColor(BLACK, panel_color_2);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->println(temperature);

  tft->setCursor(91, 81);
  tft->setTextColor(BLACK, panel_color_3);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->println(humidity);

  // print time
  tft->setFont(0);
  tft->setCursor(97, 40);
  tft->setTextColor(WHITE, BLACK);
  getLocalTime(&timeinfo);
  tft->println(&timeinfo, "%H:%M");

  return true;
}

void readWeatherRss()
{
  Serial.print("[HTTP] begin...\n");
  http.begin(WEATHER_RSS_URL);

  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();

  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  // HTTP header has been send and Server response header has been handled
  if (httpCode <= 0)
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  else
  {
    if (httpCode != HTTP_CODE_OK)
    {
      Serial.printf("[HTTP] Not OK!\n");
      delay(5000);
    }
    else
    {
      // get lenght of document (is -1 when Server sends no Content-Length header)
      len = http.getSize();
      Serial.printf("[HTTP] size: %d\n", len);

      if (len <= 0)
      {
        Serial.printf("[HTTP] Unknow content size: %d\n", len);
      }
      else
      {
        String xml = http.getString();
        int key_idx = xml.indexOf("updated at");
        int val_start_idx = key_idx + 10;
        int val_end_idx = xml.indexOf(':', val_start_idx);
        int update_hour = xml.substring(val_start_idx, val_end_idx).toInt();
        key_idx = xml.indexOf("Air temperature");
        val_start_idx = xml.indexOf(':', key_idx) + 1;
        val_end_idx = xml.indexOf("degrees", val_start_idx);
        int temperature = xml.substring(val_start_idx, val_end_idx).toInt();
        key_idx = xml.indexOf("Relative Humidity");
        val_start_idx = xml.indexOf(':', key_idx) + 1;
        val_end_idx = xml.indexOf("per", val_start_idx);
        int humidity = xml.substring(val_start_idx, val_end_idx).toInt();
        key_idx = xml.indexOf("UV Index");
        val_start_idx = xml.indexOf(':', key_idx) + 1;
        val_end_idx = xml.indexOf('\n', val_start_idx);
        int uvIdx = xml.substring(val_start_idx, val_end_idx).toInt();

        // print Observatory data
        tft->setFont(&FreeMonoBold9pt7b);

        tft->setTextColor(BLACK, panel_color_4);
        if (uvIdx < 10)
        {
          tft->setCursor(14, 141);
        }
        else
        {
          tft->setCursor(9, 141);
        }
        tft->println(uvIdx);

        tft->setCursor(50, 141);
        tft->setTextColor(BLACK, panel_color_5);
        if (temperature < 10)
        {
          tft->print(" ");
        }
        tft->println(temperature);

        tft->setCursor(91, 141);
        tft->setTextColor(BLACK, panel_color_6);
        if (humidity < 10)
        {
          tft->print(" ");
        }
        tft->println(humidity);

        // print last update time
        tft->setFont(0);
        tft->setCursor(78, 152);
        tft->setTextColor(WHITE, BLACK);
        getLocalTime(&timeinfo);
        tft->println(&timeinfo, "%H:%M");
        last_rss_update_hour = (update_hour == 23) ? -1 : update_hour;
      }
    }
  }
  http.end();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("DHT ESP32 example with tasks");

  tft->begin();
  tft->fillScreen(BLACK);

  tft->setFont(&FreeMonoBold9pt7b);
  tft->setCursor(0, 10);
  tft->setTextColor(RED);
  tft->println("Weather");
  tft->setTextColor(YELLOW);
  tft->println("  Dashboard");

  panel_color_1 = tft->color565(248, 64, 64);
  panel_color_2 = tft->color565(248, 128, 16);
  panel_color_3 = tft->color565(240, 240, 16);
  panel_color_4 = tft->color565(128, 248, 16);
  panel_color_5 = tft->color565(16, 248, 128);
  panel_color_6 = tft->color565(64, 64, 248);

  tft->fillRoundRect(0, 50, 40, 40, 5, panel_color_1);
  tft->fillRoundRect(44, 50, 40, 40, 5, panel_color_2);
  tft->fillRoundRect(88, 50, 40, 40, 5, panel_color_3);

  tft->fillRoundRect(0, 110, 40, 40, 5, panel_color_4);
  tft->fillRoundRect(44, 110, 40, 40, 5, panel_color_5);
  tft->fillRoundRect(88, 110, 40, 40, 5, panel_color_6);

  tft->drawCircle(75, 70, 2, BLACK);
  tft->drawCircle(75, 130, 2, BLACK);

  tft->setTextColor(BLACK);
  tft->setCursor(113, 81);
  tft->print("%");
  tft->setCursor(113, 141);
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
  tft->setCursor(2, 115);
  tft->println("UV Idx");
  tft->setCursor(52, 115);
  tft->println("Temp");
  tft->setCursor(96, 115);
  tft->println("Humi");

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
  pinMode(AIR_PIN, INPUT);
  initTemp();

  WiFi.begin(SSID_NAME, SSID_PASSWORD);

  // get time
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "NTP time: %A, %B %d %Y %H:%M:%S");

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

  getLocalTime(&timeinfo);
  // RSS update interval: "Around 2 minutes past each hour and as necessary"
  if ((timeinfo.tm_hour > last_rss_update_hour + 1) || ((timeinfo.tm_hour > last_rss_update_hour) && (timeinfo.tm_min > 7)))
  {
    readWeatherRss();
  }

  yield();
}
