#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

 extern float glob_temperature;
 extern float glob_humidity;

typedef struct {
    float temperature;
    float humidity;
} SensorData;

typedef struct 
{
    float temperature;
    float humidity;
    float inference_result;
    bool anomaly_detected;
    String anomaly_type;
    /* data */
} MLResult;

extern String WIFI_SSID;
extern String WIFI_PASS;

extern String wifi_ssid;
extern String wifi_password;
extern String ssid;
extern String password;
extern boolean isWifiConnected;

extern SemaphoreHandle_t xBinarySemaphoreInternet;


extern QueueHandle_t xQueueForLedBlink;
extern QueueHandle_t xQueueForNeoPixel;
extern QueueHandle_t xQueueForTinyML;

extern QueueHandle_t xQueueForMainServer;
extern SemaphoreHandle_t xSemaphoreMutex;

extern QueueHandle_t xQueueTempHumiForMain;
extern QueueHandle_t xQueueLatestState;

extern QueueHandle_t xQueueForIoT;
#endif