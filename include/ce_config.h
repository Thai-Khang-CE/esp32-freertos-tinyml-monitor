#ifndef CE_CONFIG_H
#define CE_CONFIG_H

/*============================================================
  SMART HOME CE FIRMWARE - CONFIGURATION
  Platform: ESP32 YOLO UNO
  Author: CE Team (Khang + Dat)
  ============================================================*/

// ============================================================
// HARDWARE PINS
// ============================================================
#define SENSOR_SDA_PIN      21    // DHT20 I2C SDA
#define SENSOR_SCL_PIN      22    // DHT20 I2C SCL
#define RELAY_LIGHT_PIN     12    // Light relay GPIO
#define RELAY_FAN_PIN       13    // Fan relay GPIO
#define I2C_SPEED          100000 // I2C clock speed (100kHz)

// ============================================================
// FREERTOS TASK CONFIGURATION
// ============================================================
#define SENSOR_TASK_STACK   2048
#define SENSOR_TASK_PRIO    3     // High priority
#define SENSOR_READ_INTERVAL 5000 // ms (5 seconds)

#define RELAY_TASK_STACK    2048
#define RELAY_TASK_PRIO     1     // Low priority
#define RELAY_DEBOUNCE_MS   100   // Anti-bounce delay

#define TINYML_TASK_STACK   4096
#define TINYML_TASK_PRIO    2     // Medium priority

#define HTTP_UPLOAD_TASK_STACK  4096
#define HTTP_UPLOAD_TASK_PRIO   2
#define HTTP_UPLOAD_INTERVAL    30000 // ms (30 seconds)

#define HTTP_COMMAND_TASK_STACK 4096
#define HTTP_COMMAND_TASK_PRIO  2
#define HTTP_COMMAND_INTERVAL   10000 // ms (10 seconds)

#define WIFI_TASK_STACK     3072
#define WIFI_TASK_PRIO      1

// ============================================================
// QUEUE SIZES
// ============================================================
#define QUEUE_SENSOR_SIZE   16
#define QUEUE_RELAY_SIZE    8
#define QUEUE_STATUS_SIZE   4

// ============================================================
// WIFI CONFIGURATION
// ============================================================
#define CE_WIFI_SSID           "Your_WiFi_SSID"
#define CE_WIFI_PASSWORD       "Your_Password"
#define CE_WIFI_CONNECT_TIMEOUT 15000 // ms
#define CE_WIFI_RECONNECT_INTERVALS {5000, 10000, 20000, 30000} // Exponential backoff

// ============================================================
// BACKEND API CONFIGURATION
// ============================================================
#define CE_BACKEND_URL         "http://192.168.1.50:3000"
#define CE_DEVICE_ID           "esp32-01"
#define CE_DEVICE_SECRET       ""  // Empty = no auth header

// API Endpoints
#define API_SENSORS_DATA    "/api/sensors/data"
#define API_DEVICES_COMMAND "/api/devices/command"
#define API_DEVICES_STATUS  "/api/devices/status"

// HTTP Configuration
#define HTTP_TIMEOUT_MS     5000  // Request timeout
#define HTTP_RETRY_COUNT    3     // Retry attempts
#define HTTP_RETRY_DELAY_MS 5000  // Delay between retries

// ============================================================
// SENSOR CONFIGURATION
// ============================================================
#define SENSOR_TEMP_MIN     -40.0f
#define SENSOR_TEMP_MAX     85.0f
#define SENSOR_HUMIDITY_MIN 0.0f
#define SENSOR_HUMIDITY_MAX 100.0f



// ============================================================
// RELAY CONFIGURATION
// ============================================================
#define RELAY_ON_STATE      HIGH   // Active HIGH
#define RELAY_OFF_STATE     LOW
#define RELAY_VERIFICATION_RETRIES 3

// ============================================================
// SERIAL DEBUG
// ============================================================
#define SERIAL_BAUD         115200
#define DEBUG_ENABLED       1

#if DEBUG_ENABLED
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

// ============================================================
// MISC
// ============================================================
#define HEALTH_CHECK_INTERVAL 60000 // Print health metrics every 60s
#define MAX_UPTIME_SECONDS  (365 * 24 * 60 * 60)  // 1 year (for overflow protection)

#endif // CE_CONFIG_H
