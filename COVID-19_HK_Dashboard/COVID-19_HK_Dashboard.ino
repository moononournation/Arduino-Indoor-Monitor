/* WiFi settings */
#include <WiFi.h>
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 28800L  // Timezone +0800
#define DAYLIGHT_OFFSET_SEC 0L // no daylight saving
#define UPDATE_INTERVAL 10     // sensors update interval in seconds

const char* chp_dashboard_url = "https://chp-dashboard.geodata.gov.hk/nia/en.html";
const char* chp_cases_url = "https://services8.arcgis.com/PXQv9PaDJHzt8rp0/arcgis/rest/services/LatestReport_LIM_View/FeatureServer/0/query?f=json&where=1%3D1&returnGeometry=false&spatialRel=esriSpatialRelIntersects&outFields=*&outSR=102100&resultOffset=0&resultRecordCount=50&cacheHint=true";
const char* chp_death_json_url = "https://services8.arcgis.com/PXQv9PaDJHzt8rp0/arcgis/rest/services/HKConfirmedCases_0208_View/FeatureServer/0/query?where=Hospitalised_Discharged_Decease%3D%27Deceased%27&returnCountOnly=true&f=json";
const char* chp_dischgd_json_url = "https://services8.arcgis.com/PXQv9PaDJHzt8rp0/arcgis/rest/services/HKConfirmedCases_0208_View/FeatureServer/0/query?where=Hospitalised_Discharged_Decease%3D%27Discharged%27&returnCountOnly=true&f=json";
const char* hko_weather_rss_url = "http://rss.weather.gov.hk/rss/CurrentWeather.xml";

// HTTPS howto: https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
const char* gov_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

const char* arcgis_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

/* display settings */
#include <Arduino_GFX_Library.h>
#include "FreeMonoBold9pt7b.h"
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
static uint16_t dark_dark_grey;
static uint16_t panel_color_01;
static uint16_t panel_color_02;
static uint16_t panel_color_03;
static uint16_t panel_color_04;
static uint16_t panel_color_05;
static uint16_t panel_color_06;
static uint16_t panel_color_07;
static uint16_t panel_color_08;
static uint16_t panel_color_09;
static uint16_t panel_color_10;
static uint16_t panel_color_11;
static uint16_t panel_color_12;

/**
 * pngle_on_draw
 * Callback function for load_png()
*/
void pngle_on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
  if (rgba[3]) // not transparent
  {
    tft->fillRect(x + 78, y + 110, w, h, tft->color565(rgba[0], rgba[1], rgba[2]));
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
    log_i("%s not exists in local, download from internet.", filename);

    // get URL
    http.begin(png_url);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
      log_e("HTTP ERROR: %d", httpCode);
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
  log_i("stream local file to PNG decoder");
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
      log_e("ERROR: %s\n", pngle_error(pngle));
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

String getHttpsReturnStr(const char* url, const char* root_ca) {
  HTTPClient https;
  String result;

  log_i("[HTTPS] begin...\n");
  https.begin(url, root_ca);

  log_i("[HTTPS] GET...\n");
  int httpCode = https.GET();

  log_i("[HTTPS] GET... code: %d\n", httpCode);
  // HTTP header has been send and Server response header has been handled
  if (httpCode <= 0)
  {
    log_e("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    return result;
  }

  if (httpCode != HTTP_CODE_OK)
  {
    log_e("[HTTPS] Not OK!\n");
    https.end();
    return result;
  }

  delay(100);

  result = https.getString();
  https.end();
  return result;
}

/**
 * updateChp
 * @return bool
 *    true if update success
*/
bool updateChp()
{
  getHttpsReturnStr(chp_dashboard_url, gov_root_ca);

  String json = getHttpsReturnStr(chp_cases_url, arcgis_root_ca);
  log_d("return: %s", json.c_str());

  // confirmed cases count
  int key_idx = json.indexOf("features");
  key_idx = json.indexOf("Number_of_confirmed_cases", key_idx);
  int val_start_idx = json.indexOf(':', key_idx) + 1;
  int val_end_idx = json.indexOf(",", val_start_idx);
  int confirmed_count = json.substring(val_start_idx, val_end_idx).toInt();
  // ruled out cases count
  key_idx = json.indexOf("Number_of_ruled_out_cases", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int ruled_out_count = json.substring(val_start_idx, val_end_idx).toInt();
  // Investigation cases count
  key_idx = json.indexOf("Number_of_cases_still_hospitali", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int investigation_count = json.substring(val_start_idx, val_end_idx).toInt();
  // reported cases count
  key_idx = json.indexOf("Number_of_cases_fulfilling_the_", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int reported_count = json.substring(val_start_idx, val_end_idx).toInt();

  json = getHttpsReturnStr(chp_death_json_url, arcgis_root_ca);
  log_d("return: %s", json.c_str());

  // death count
  key_idx = json.indexOf("count");
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf("}", val_start_idx);
  int death_count = json.substring(val_start_idx, val_end_idx).toInt();

  json = getHttpsReturnStr(chp_dischgd_json_url, arcgis_root_ca);
  log_d("return: %s", json.c_str());

  // death count
  key_idx = json.indexOf("count");
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf("}", val_start_idx);
  int dischgd_count = json.substring(val_start_idx, val_end_idx).toInt();

  // print CHP data
  // confirmed cases count
  tft->setTextColor(panel_color_01, dark_dark_grey);
  if (confirmed_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(9, 42);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (confirmed_count >= 100)
    {
      tft->setCursor(2, 50);
    }
    else if (confirmed_count >= 10)
    {
      tft->setCursor(8, 50);
    }
    else
    {
      tft->setCursor(13, 50);
    }
  }
  tft->print(confirmed_count);
  // death count
  tft->setTextColor(panel_color_02, dark_dark_grey);
  if (death_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(53, 42);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (death_count >= 100)
    {
      tft->setCursor(45, 50);
    }
    else if (death_count >= 10)
    {
      tft->setCursor(51, 50);
    }
    else
    {
      tft->setCursor(56, 50);
    }
  }
  tft->print(death_count);
  // dischgd count
  tft->setTextColor(panel_color_03, dark_dark_grey);
  if (dischgd_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(96, 42);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (dischgd_count >= 100)
    {
      tft->setCursor(88, 50);
    }
    else if (dischgd_count >= 10)
    {
      tft->setCursor(94, 50);
    }
    else
    {
      tft->setCursor(99, 50);
    }
  }
  tft->print(dischgd_count);
  // Investigation cases count
  tft->setTextColor(panel_color_04, dark_dark_grey);
  if (investigation_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(9, 84);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (investigation_count >= 100)
    {
      tft->setCursor(2, 92);
    }
    else if (investigation_count >= 10)
    {
      tft->setCursor(8, 92);
    }
    else
    {
      tft->setCursor(13, 92);
    }
  }
  tft->print(investigation_count);
  // ruled out cases count
  tft->setTextColor(panel_color_05, dark_dark_grey);
  if (ruled_out_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(53, 84);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (ruled_out_count >= 100)
    {
      tft->setCursor(45, 92);
    }
    else if (ruled_out_count >= 10)
    {
      tft->setCursor(51, 92);
    }
    else
    {
      tft->setCursor(56, 92);
    }
  }
  tft->print(ruled_out_count);
  // reported cases count
  tft->setTextColor(panel_color_06, dark_dark_grey);
  if (reported_count >= 1000)
  {
    tft->setFont(0);
    tft->setCursor(96, 84);
  } else {
    tft->setFont(&FreeMonoBold9pt7b);
    if (reported_count >= 100)
    {
      tft->setCursor(88, 92);
    }
    else if (reported_count >= 10)
    {
      tft->setCursor(94, 92);
    }
    else
    {
      tft->setCursor(99, 92);
    }
  }
  tft->print(reported_count);

  tft->setFont(0);
  return true;
}

/**
 * updateHko
 * Reads RSS feed from HK Observatory
 * @return bool
 *    true if update success
*/
bool updateHko()
{
  log_i("[HTTP] begin...\n");
  http.begin(hko_weather_rss_url);

  log_i("[HTTP] GET...\n");
  int httpCode = http.GET();

  log_i("[HTTP] GET... code: %d\n", httpCode);
  // HTTP header has been send and Server response header has been handled
  if (httpCode <= 0)
  {
    log_i("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }

  if (httpCode != HTTP_CODE_OK)
  {
    log_i("[HTTP] Not OK!\n");
    http.end();
    return false;
  }

  // get length of document (is -1 when Server sends no Content-Length header)
  int len = http.getSize();
  log_i("[HTTP] size: %d\n", len);

  if (len <= 0)
  {
    log_i("[HTTP] Unknow content size: %d\n", len);
    http.end();
    return false;
  }

  // get XML string
  String xml = http.getString();
  log_d("return: %s", xml.c_str());
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
  // UV Index
  tft->setTextColor(BLACK, panel_color_07);
  if (uvIdx < 10)
  {
    tft->setCursor(9, 121);
  }
  else
  {
    tft->setCursor(6, 121);
  }
  tft->print(uvIdx);

  // Air temperature
  tft->setCursor(29, 121);
  tft->setTextColor(BLACK, panel_color_08);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->print(temperature);

  // Relative Humidity
  tft->setCursor(55, 121);
  tft->setTextColor(BLACK, panel_color_09);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->print(humidity);

  // print last update time
  tft->setCursor(97, 101);
  tft->setTextColor(DARKGREY, BLACK);
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
    tft->setCursor(47, 131);
    tft->setTextColor(DARKGREY, BLACK);
    tft->print(&timeinfo, "%H:%M");
  }

  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0)
  {
    log_i("DHT11 error status: %s", dht.getStatusString());
    return false;
  }

  float air_quality = analogRead(AIR_PIN) * 10.0 / 4096.0;
  int temperature = newValues.temperature;
  int humidity = newValues.humidity;

  // print indoor data
  // Air Quality
  tft->setCursor(3, 151);
  tft->setTextColor(BLACK, panel_color_10);
  tft->print(air_quality, 1);

  // Air temperature
  tft->setCursor(29, 151);
  tft->setTextColor(BLACK, panel_color_11);
  if (temperature < 10)
  {
    tft->print(" ");
  }
  tft->print(temperature);

  // Relative Humidity
  tft->setCursor(55, 151);
  tft->setTextColor(BLACK, panel_color_12);
  if (humidity < 10)
  {
    tft->print(" ");
  }
  tft->print(humidity);

  return true;
}

void setup()
{
  // put your setup code here, to run once:

  log_i("Novel Coronavirus Hong Kong Dashboard");

  // Connect WiFi
  WiFi.begin(SSID_NAME, SSID_PASSWORD);

  // Initialize temperature sensor
  dht.setup(DHT_PIN, DHTesp::DHT11);
  log_i("DHT initiated");

  // Initialize display
  tft->begin();
  tft->fillScreen(BLACK);

  // print WiFi MAC address
  String mac = WiFi.macAddress();
  log_i("MAC address: %s", mac.c_str());
  tft->setTextColor(tft->color565(0, 16, 0));
  tft->setCursor(79, 110);
  tft->print(mac.substring(0, 8));
  tft->setCursor(79, 120);
  tft->print(mac.substring(9, 17));

  // print banner
  tft->setFont(&FreeMonoBold9pt7b);
  tft->setTextColor(WHITE);
  tft->setCursor(0, 12);
  tft->print("COVID-19 HK");
  tft->setFont(0);

  // Initialize panel colors
  dark_dark_grey = tft->color565(8, 4, 8);
  panel_color_01 = tft->color565(255, 0, 0);
  panel_color_02 = tft->color565(128, 128, 128);
  panel_color_03 = tft->color565(16, 16, 255);
  panel_color_04 = tft->color565(255, 255, 0);
  panel_color_05 = tft->color565(0, 255, 0);
  panel_color_06 = tft->color565(255, 255, 255);
  panel_color_07 = tft->color565(248, 16, 248);
  panel_color_08 = tft->color565(248, 64, 64);
  panel_color_09 = tft->color565(248, 128, 16);
  panel_color_10 = tft->color565(128, 248, 16);
  panel_color_11 = tft->color565(16, 248, 128);
  panel_color_12 = tft->color565(64, 64, 248);

  // Draw panels
  tft->fillRect(0, 16, 42, 40, dark_dark_grey);
  tft->fillRect(43, 16, 42, 40, dark_dark_grey);
  tft->fillRect(86, 16, 42, 40, dark_dark_grey);
  tft->fillRect(0, 57, 42, 40, dark_dark_grey);
  tft->fillRect(43, 57, 42, 40, dark_dark_grey);
  tft->fillRect(86, 57, 42, 40, dark_dark_grey);

  tft->fillRoundRect(0, 110, 24, 20, 4, panel_color_07);
  tft->fillRoundRect(26, 110, 24, 20, 4, panel_color_08);
  tft->fillRoundRect(52, 110, 24, 20, 4, panel_color_09);
  tft->fillRoundRect(0, 140, 24, 20, 4, panel_color_10);
  tft->fillRoundRect(26, 140, 24, 20, 4, panel_color_11);
  tft->fillRoundRect(52, 140, 24, 20, 4, panel_color_12);

  // Draw unit signs
  tft->drawCircle(43, 122, 1, BLACK);
  tft->drawCircle(43, 152, 1, BLACK);
  tft->setTextColor(BLACK);
  tft->setCursor(69, 121);
  tft->print("%");
  tft->setCursor(69, 151);
  tft->print("%");

  // print labels
  tft->setTextColor(WHITE);
  tft->setCursor(0, 101);
  tft->print("HKO Weather");
  tft->setCursor(0, 131);
  tft->print("Indoor");

  tft->setTextColor(panel_color_01);
  tft->setCursor(0, 17);
  tft->print("Confirm");
  tft->setCursor(6, 25);
  tft->print("Cases");
  tft->setTextColor(panel_color_02);
  tft->setCursor(49, 17);
  tft->print("Death");
  tft->setTextColor(panel_color_03);
  tft->setCursor(86, 17);
  tft->print("Dischgd");
  tft->setTextColor(panel_color_04);
  tft->setCursor(0, 58);
  tft->print("Invstg.");
  tft->setCursor(6, 66);
  tft->print("Cases");
  tft->setTextColor(panel_color_05);
  tft->setCursor(49, 58);
  tft->print("Ruled");
  tft->setCursor(55, 66);
  tft->print("Out");
  tft->setTextColor(panel_color_06);
  tft->setCursor(86, 58);
  tft->print("Reportd");
  tft->setCursor(92, 66);
  tft->print("Cases");

  tft->setTextColor(dark_dark_grey);
  tft->setCursor(7, 111);
  tft->print("UV");
  tft->setCursor(29, 111);
  tft->print("Tem");
  tft->setCursor(55, 111);
  tft->print("Hum");
  tft->setCursor(7, 141);
  tft->print("AQ");
  tft->setCursor(29, 141);
  tft->print("Tem");
  tft->setCursor(55, 141);
  tft->print("Hum");

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
    log_i("SPIFFS init Failed");
    return;
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  // Update indoor sensors data
  updateIndoorData();

  // HK Observatory RSS update interval: "Around 2 minutes past each hour and as necessary"
  if (getLocalTime(&timeinfo))
  {
    if ((last_rss_update_hour == -2) || (timeinfo.tm_hour > last_rss_update_hour + 1) || ((timeinfo.tm_hour > last_rss_update_hour) && (timeinfo.tm_min > 7)))
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        updateChp();
        updateHko();
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
          updateChp();
          updateHko();
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
