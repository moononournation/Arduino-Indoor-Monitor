#include <SPI.h>
#include <Arduino_HWSPI.h>
#include <Arduino_GFX.h>     // Core graphics library by Adafruit
#include <Arduino_ST7735.h>  // Hardware-specific library for ST7735 (with or without CS pin)
#include "FreeSerif9pt7b.h"
#include "FreeMonoBold9pt7b.h"
#include <DHTesp.h>
#include <Ticker.h>

#define TFT_BL 22

Arduino_HWSPI *bus = new Arduino_HWSPI(16 /* DC */, 5 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */);
Arduino_ST7735 *tft = new Arduino_ST7735(bus, 17 /* RST */, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);

DHTesp dht;

void tempTask(void *pvParameters);
void getAir();
bool getTemperature();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
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
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
	dht.setup(dhtPin, DHTesp::DHT11);
	Serial.println("DHT initiated");

  // Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,                       /* Function to implement the task */
			"tempTask ",                    /* Name of the task */
			4000,                           /* Stack size in words */
			NULL,                           /* Task input parameter */
			5,                              /* Priority of the task */
			&tempTaskHandle,                /* Task handle. */
			1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
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
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
			getTemperature();
		}
    // Got sleep again
		vTaskSuspend(NULL);
	}
}

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool getTemperature() {
	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		return false;
	}

	float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  // Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);

  float air_quality = analogRead(airPin) * 100.0 / 4096.0;

  Serial.println(String(air_quality) + " " + String(newValues.temperature) + " " + String(newValues.humidity));

  tft->fillRect(0, 36, 128, 124, BLACK);

  tft->setTextColor(GREEN);
  tft->setFont(&FreeSerif9pt7b);
  tft->setCursor(0, 50);
  tft->println("Air Quality");
  tft->setFont(&FreeMonoBold9pt7b);
  tft->print("      ");
  if (air_quality < 10) {
    tft->print(" ");
  }
  tft->println(air_quality);

  tft->setTextColor(CYAN);
  tft->setFont(&FreeSerif9pt7b);
  tft->println("Temperature");
  tft->setFont(&FreeMonoBold9pt7b);
  tft->print("      ");
  tft->println(newValues.temperature);

  tft->setTextColor(BLUE);
  tft->setFont(&FreeSerif9pt7b);
  tft->println("Humidity");
  tft->setFont(&FreeMonoBold9pt7b);
  tft->print("      ");
  tft->println(newValues.humidity);

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
  tft->println("Indoor");
  tft->setTextColor(YELLOW);
  tft->println("    Monitor");
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

  pinMode(airPin, INPUT);
  initTemp();

  // Signal end of setup() to tasks
  tasksEnabled = true;
}

void loop() {
  if (!tasksEnabled) {
    // Wait 2 seconds to let system settle down
    delay(2000);
    // Enable task that will read values from the DHT sensor
    tasksEnabled = true;
    if (tempTaskHandle != NULL) {
			vTaskResume(tempTaskHandle);
		}
  }
  yield();
}
