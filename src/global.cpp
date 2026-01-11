#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;


String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "Hudahadu";
String wifi_password = "khotinhv";
boolean isWifiConnected = false;


SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();
SemaphoreHandle_t xSemaphoreMutex = xSemaphoreCreateMutex();

QueueHandle_t xQueueForLedBlink = xQueueCreate(5, sizeof(SensorData));
QueueHandle_t xQueueForNeoPixel = xQueueCreate(5, sizeof(SensorData));
QueueHandle_t xQueueForTinyML   = xQueueCreate(5, sizeof(SensorData));
QueueHandle_t xQueueForMainServer = xQueueCreate(5, sizeof(MLResult));
QueueHandle_t xQueueTempHumiForMain = xQueueCreate(5, sizeof(SensorData));
QueueHandle_t xQueueLatestState = xQueueCreate(1, sizeof(MLResult));
QueueHandle_t xQueueForIoT = xQueueCreate(5, sizeof(SensorData));

