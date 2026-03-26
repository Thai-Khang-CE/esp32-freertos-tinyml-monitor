#ifndef CE_GLOBAL_H
#define CE_GLOBAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "ce_data_types.h"

/*============================================================
  GLOBAL VARIABLES & IPC PRIMITIVES FOR CE FIRMWARE
  ============================================================*/

// ============================================================
// QUEUES FOR INTER-TASK COMMUNICATION
// ============================================================

// Sensor readings: SensorTask -> TinyMLTask, HttpUploadTask
extern QueueHandle_t xQueueSensorData;

// Relay commands: CommandPollTask -> RelayTask
extern QueueHandle_t xQueueCommand;

// Device status: RelayTask -> HttpUploadTask
extern QueueHandle_t xQueueStatus;

// ============================================================
// SEMAPHORES & MUTEXES
// ============================================================

// HTTP Mutex: Ensure only one HTTP request at a time
extern SemaphoreHandle_t xSemaphoreHTTP;

// WiFi connection binary semaphore
extern SemaphoreHandle_t xSemaphoreWiFi;

// ============================================================
// GLOBAL STATE
// ============================================================

// WiFi state
extern volatile bool g_wifiConnected;
extern volatile int8_t g_wifiSignal;     // RSSI (dBm)
extern volatile uint32_t g_wifiUptime;   // ms since connected

// Device state
extern volatile bool g_lightOn;
extern volatile bool g_fanOn;
extern volatile char g_lastCommandId[32];

// Sensor state
extern volatile float g_lastTemperature;
extern volatile float g_lastHumidity;
extern volatile bool g_sensorValid;
extern volatile float g_lastAnomalyScore;

// System state
extern volatile uint32_t g_systemUptime; // seconds
extern volatile uint32_t g_freeHeap;    // bytes
extern volatile uint8_t g_errorCount;

// ============================================================
// LEGACY GLOBALS (from old project) - KEEP FOR COMPATIBILITY
// ============================================================

extern float glob_temperature;
extern float glob_humidity;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

// Legacy queues (old project)
extern QueueHandle_t xQueueForLedBlink;
extern QueueHandle_t xQueueForNeoPixel;
extern QueueHandle_t xQueueForTinyML;
extern QueueHandle_t xQueueForMainServer;
extern QueueHandle_t xQueueTempHumiForMain;
extern QueueHandle_t xQueueLatestState;
extern QueueHandle_t xQueueForIoT;

// Legacy semaphores (old project)
extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern SemaphoreHandle_t xSemaphoreMutex;

// ============================================================
// INITIALIZATION FUNCTION
// ============================================================

/**
 * @brief Initialize all CE global queues and semaphores
 * Called from setup() before task creation
 */
void ce_globals_init(void);

/**
 * @brief Print health metrics to serial
 */
void ce_print_health_metrics(void);

#endif // CE_GLOBAL_H
