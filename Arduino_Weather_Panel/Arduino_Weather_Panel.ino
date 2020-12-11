/* WiFi settings */
#include <WiFi.h>
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 28800L  // Timezone +0800
#define DAYLIGHT_OFFSET_SEC 0L // no daylight saving
#define UPDATE_INTERVAL 10     // sensors update interval in seconds
#define WEATHER_RSS_URL "http://rss.weather.gov.hk/rss/CurrentWeather.xml"

/* display settings */
#include <Arduino_GFX_Library.h>
#include "FreeMonoBold9pt7b.h" // font for display
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(16 /* DC */, 5 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */);
Arduino_ST7735 *tft = new Arduino_ST7735(bus, 17 /* RST */, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);
#define TFT_BL 22

/* sensors settings */
#include <DHTesp.h>
static DHTesp dht;
#define AIR_PIN 34
#define DHT_PIN 21

/* HTTPClient library */
#include <HTTPClient.h>
static HTTPClient http;

/* time library */
#include <time.h>
static struct tm timeinfo;

/* decode PNG library */
#include <pngle.h>

/* SPIFFS temp file system for storing image cache */
#include "FS.h"
#include "SPIFFS.h"

/* static variables */
static int8_t last_rss_update_hour = -2; // never updated
static char png_url[128];
static uint16_t panel_color_1;
static uint16_t panel_color_2;
static uint16_t panel_color_3;
static uint16_t panel_color_4;
static uint16_t panel_color_5;
static uint16_t panel_color_6;

/**
 * pngle_on_draw
 * Callback function for load_png()
*/
void pngle_on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
  if (rgba[3]) // not transparent
  {
    tft->fillRect(x + 78, y, w, h, tft->color565(rgba[0], rgba[1], rgba[2]));
  }
}

/**
 * load_png
 * Reads and displays PNG image from internet
 * @return bool
 *    true if load success
*/
bool load_png(String png_url)
{
  int key_idx = png_url.indexOf("img/");
  int val_start_idx = key_idx + 3;
  int val_end_idx = png_url.length();
  String filename = png_url.substring(val_start_idx, val_end_idx);

  File file;
  uint8_t buf[2048];
  char *buff = (char *)buf;
  size_t size;
  int len;

  if (!SPIFFS.exists(filename))
  {
    Serial.println(filename + " not exists in local, download from internet.");

    // get URL
    http.begin(png_url);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
      Serial.printf("HTTP ERROR: %d\n", httpCode);
      http.end();
      return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    file = SPIFFS.open(filename, FILE_WRITE);
    while (http.connected())
    {
      size_t size = stream->available();
      if (!size)
      {
        delay(1);
      }
      else
      {
        if (size > sizeof(buf)) {
          size = sizeof(buf);
        }
        len = stream->readBytes(buf, size);
        file.write(buf, len);
      }
    }

    http.end();
    file.flush();
    file.close();
  }

  // file stream to PNG decoder
  Serial.println("stream local file to PNG decoder");
  file = SPIFFS.open(filename, FILE_READ);
  pngle_t *pngle = pngle_new();
  pngle_set_draw_callback(pngle, pngle_on_draw);
  int remain = 0;
  while (size = file.available())
  {
    if (size > sizeof(buf) - remain)
    {
      size = sizeof(buf) - remain;
    }

    len = file.readBytes(buff + remain, size);
    int fed = pngle_feed(pngle, buff, remain + len);
    if (fed < 0)
    {
      Serial.printf("ERROR: %s\n", pngle_error(pngle));
      break;
    }

    remain = remain + len - fed;
    if (remain > 0)
      memmove(buff, buff + fed, remain);
  }

  pngle_destroy(pngle);
  file.close();

  return true;
}

/**
 * updateRss
 * Reads RSS feed from HK Observatory
 * @return bool
 *    true if update success
*/
bool updateRss()
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
    http.end();
    return false;
  }

  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[HTTP] Not OK!\n");
    http.end();
    return false;
  }

  // get length of document (is -1 when Server sends no Content-Length header)
  int len = http.getSize();
  Serial.printf("[HTTP] size: %d\n", len);

  if (len <= 0)
  {
    Serial.printf("[HTTP] Unknow content size: %d\n", len);
    http.end();
    return false;
  }

  // get XML string
  String xml = http.getString();
  http.end();

  // update hour
  int key_idx = xml.indexOf("updated at");
  int val_start_idx = key_idx + 11;
  int val_end_idx = xml.indexOf(':', val_start_idx);
  int update_hour = xml.substring(val_start_idx, val_end_idx).toInt();
  String update_time = xml.substring(val_end_idx - 2, val_end_idx + 3);
  // weather image
  key_idx = xml.indexOf("img", val_end_idx);
  val_start_idx = xml.indexOf('"', key_idx) + 1;
  val_end_idx = xml.indexOf('"', val_start_idx);
  String png_url = xml.substring(val_start_idx, val_end_idx);
  // Air temperature
  key_idx = xml.indexOf("Air temperature");
  val_start_idx = xml.indexOf(':', key_idx) + 1;
  val_end_idx = xml.indexOf("degrees", val_start_idx);
  int temperature = xml.substring(val_start_idx, val_end_idx).toInt();
  // Relative Humidity
  key_idx = xml.indexOf("Relative Humidity");
  val_start_idx = xml.indexOf(':', key_idx) + 1;
  val_end_idx = xml.indexOf("per", val_start_idx);
  int humidity = xml.substring(val_start_idx, val_end_idx).toInt();
  // UV Index
  key_idx = xml.indexOf("UV Index");
  val_start_idx = xml.indexOf(':', key_idx) + 1;
  val_end_idx = xml.indexOf('\n', val_start_idx);
  int uvIdx = xml.substring(val_start_idx, val_end_idx).toInt();

  // print Observatory data
  tft->setFont(&FreeMonoBold9pt7b);

  // UV Index
  tft->setTextColor(BLACK, panel_color_1);
  if (uvIdx < 10)
  {
    tft->setCursor(14, 93);
  }
  else
  {
    tft->setCursor(9, 93);
  }
  tft->print(uvIdx);

  // Air temperature
  tft->setCursor(50, 93);
  tft->setTextColor(BLACK, panel_color_2);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->print(temperature);

  // Relative Humidity
  tft->setCursor(91, 93);
  tft->setTextColor(BLACK, panel_color_3);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->print(humidity);

  // print last update time
  tft->setFont(0);
  tft->setCursor(97, 52);
  tft->setTextColor(LIGHTGREY, BLACK);
  tft->print(update_time);
  last_rss_update_hour = (update_hour == 23) ? -1 : update_hour;

  // load weather PNG icon
  return load_png(png_url);
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
  if (getLocalTime(&timeinfo))
  {
    // print time
    tft->setFont(0);
    tft->setCursor(97, 108);
    tft->setTextColor(LIGHTGREY, BLACK);
    tft->print(&timeinfo, "%H:%M");
  }

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

  // print indoor data
  tft->setFont(&FreeMonoBold9pt7b);

  // Air Quality
  tft->setCursor(3, 149);
  tft->setTextColor(BLACK, panel_color_4);
  tft->print(air_quality, 1);

  // Air temperature
  tft->setCursor(50, 149);
  tft->setTextColor(BLACK, panel_color_5);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->print(temperature);

  // Relative Humidity
  tft->setCursor(91, 149);
  tft->setTextColor(BLACK, panel_color_6);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->print(humidity);

  // for Serial Plotter
  Serial.println(String(air_quality) + " " + String(temperature) + " " + String(humidity));

  return true;
}

void setup()
{
  // put your setup code here, to run once:

  // Initialize serial port
  Serial.begin(115200);
  Serial.println();
  Serial.println("Arduino Weather Panel");

  // Connect WiFi
  WiFi.begin(SSID_NAME, SSID_PASSWORD);

  // Initialize temperature sensor
  dht.setup(DHT_PIN, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Initialize display
  tft->begin();
  tft->fillScreen(BLACK);

  // print WiFi MAC address
  String mac = WiFi.macAddress();
  Serial.println(mac);
  tft->setTextColor(tft->color565(0, 16, 0));
  tft->setCursor(79, 0);
  tft->print(mac.substring(0, 8));
  tft->setCursor(79, 10);
  tft->print(mac.substring(9, 17));

  // print banner
  tft->setFont(&FreeMonoBold9pt7b);
  tft->setCursor(0, 22);
  tft->setTextColor(RED);
  tft->print("Weather");
  tft->setCursor(22, 42);
  tft->setTextColor(YELLOW);
  tft->println("Panel");

  // Initialize panel colors
  panel_color_1 = tft->color565(248, 16, 248);
  panel_color_2 = tft->color565(248, 64, 64);
  panel_color_3 = tft->color565(248, 128, 16);
  panel_color_4 = tft->color565(128, 248, 16);
  panel_color_5 = tft->color565(16, 248, 128);
  panel_color_6 = tft->color565(64, 64, 248);

  // Draw panels
  tft->fillRoundRect(0, 62, 40, 40, 5, panel_color_1);
  tft->fillRoundRect(44, 62, 40, 40, 5, panel_color_2);
  tft->fillRoundRect(88, 62, 40, 40, 5, panel_color_3);
  tft->fillRoundRect(0, 118, 40, 40, 5, panel_color_4);
  tft->fillRoundRect(44, 118, 40, 40, 5, panel_color_5);
  tft->fillRoundRect(88, 118, 40, 40, 5, panel_color_6);

  // Draw unit signs
  tft->drawCircle(75, 82, 2, BLACK);
  tft->drawCircle(75, 138, 2, BLACK);
  tft->setTextColor(BLACK);
  tft->setCursor(113, 93);
  tft->print("%");
  tft->setCursor(113, 149);
  tft->print("%");

  // print labels
  tft->setFont(0);
  tft->setTextColor(WHITE);
  tft->setCursor(0, 52);
  tft->println("RSS");
  tft->setTextColor(LIGHTGREY);
  tft->setCursor(68, 52);
  tft->println("Upd:");
  tft->setTextColor(BLACK);
  tft->setCursor(2, 67);
  tft->println("UV Idx");
  tft->setCursor(52, 67);
  tft->println("Temp");
  tft->setCursor(96, 67);
  tft->println("Humi");
  tft->setTextColor(WHITE);
  tft->setCursor(0, 108);
  tft->println("Indoor");
  tft->setTextColor(LIGHTGREY);
  tft->setCursor(68, 108);
  tft->println("Upd:");
  tft->setTextColor(BLACK);
  tft->setCursor(5, 123);
  tft->println("Air Q");
  tft->setCursor(52, 123);
  tft->println("Temp");
  tft->setCursor(96, 123);
  tft->println("Humi");

  // Turn on display backlight after all drawings ready
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
  pinMode(AIR_PIN, INPUT);

  // Initialize NTP settings
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS init Failed");
    return;
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  // Update indoor sensors data
  updateIndoorData();

  // HK Observatory RSS update interval: "Around 2 minutes past each hour and as necessary"
  if (last_rss_update_hour == -2)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      updateRss();
    }
  }
  else if (getLocalTime(&timeinfo))
  {
    if ((timeinfo.tm_hour > last_rss_update_hour + 1) || ((timeinfo.tm_hour > last_rss_update_hour) && (timeinfo.tm_min > 7)))
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        updateRss();
      }
      else
      {
        // reconnect WiFi
        WiFi.begin(SSID_NAME, SSID_PASSWORD);
        int wait_count = 0;
        while ((WiFi.status() != WL_CONNECTED) && (wait_count < 10))
        {
          delay(500);
          wait_count++;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          updateRss();
        }
      }
    }
  }

  // let system do background task
  yield();

  if (getLocalTime(&timeinfo))
  {
    // sleep a while for next update
    esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL * 1000000); // seconds to nanoseconds
    esp_light_sleep_start();
  }
  else
  {
    delay(UPDATE_INTERVAL * 1000); // seconds to milliseconds
  }
}
