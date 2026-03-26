#include "ce_sensor_task.h"
#include "ce_global.h"
#include "DHT20.h"
#include <cmath>

/*============================================================
  SENSOR TASK IMPLEMENTATION
  ============================================================*/

// DHT20 sensor instance (static, initialized on first use)
static DHT20 *dht20_ptr = nullptr;

// Sensor state for filtering
struct {
    float temp_smoothed = 0.0f;
    float hum_smoothed = 0.0f;
    float temp_readings[FILTER_OUTLIER_WINDOW];
    float hum_readings[FILTER_OUTLIER_WINDOW];
    uint8_t read_index = 0;
    uint32_t error_count = 0;
} sensor_state;

/**
 * @brief Initialize DHT20 sensor over I2C
 */
bool sensor_task_init(void) {
    Serial.println("[SENSOR] Initializing DHT20...");
    
    // Initialize I2C
    Wire.begin(SENSOR_SDA_PIN, SENSOR_SCL_PIN);
    Wire.setClock(I2C_SPEED);
    
    // Create DHT20 instance
    dht20_ptr = new DHT20(&Wire);
    
    // Initialize DHT20
    if (!dht20_ptr->begin()) {
        Serial.println("[ERROR] DHT20 not found on I2C bus!");
        delete dht20_ptr;
        dht20_ptr = nullptr;
        return false;
    }
    
    // Warmup: read 5 times to stabilize
    for (int i = 0; i < 5; i++) {
        if (dht20_ptr->read() == DHT20_OK) {
            sensor_state.temp_smoothed = dht20_ptr->getTemperature();
            sensor_state.hum_smoothed = dht20_ptr->getHumidity();
            delay(100);
        }
    }
    
    Serial.println("[SENSOR] DHT20 initialized successfully");
    return true;
}

/**
 * @brief Validate sensor reading against limits
 */
bool sensor_validate(float temp, float hum) {
    if (isnan(temp) || isnan(hum)) {
        return false;
    }
    if (temp < SENSOR_TEMP_MIN || temp > SENSOR_TEMP_MAX) {
        return false;
    }
    if (hum < SENSOR_HUMIDITY_MIN || hum > SENSOR_HUMIDITY_MAX) {
        return false;
    }
    return true;
}

/**
 * @brief Exponential smoothing filter
 */
float sensor_smooth(float alpha, float raw, float smoothed) {
    // If first reading, use raw
    if (smoothed == 0.0f && raw != 0.0f) {
        return raw;
    }
    return alpha * raw + (1.0f - alpha) * smoothed;
}

/**
 * @brief Z-score outlier detection
 */
bool sensor_is_outlier(float value, float mean, float stddev, float threshold) {
    if (stddev == 0.0f) return false;
    float zscore = fabs((value - mean) / stddev);
    return zscore > threshold;
}

/**
 * @brief Calculate data quality metric
 * Quality = 1.0 - (error_count / max_errors) - outlier_factor
 */
float sensor_calc_quality(void) {
    float quality = 1.0f;
    
    // Penalty for errors
    quality -= (sensor_state.error_count * 0.05f);
    
    // Clamp to [0, 1]
    if (quality < 0.0f) quality = 0.0f;
    if (quality > 1.0f) quality = 1.0f;
    
    return quality;
}

/**
 * @brief Main sensor task - runs every SENSOR_READ_INTERVAL
 */
void sensor_task(void *parameter) {
    Serial.println("[SENSOR] Task started");
    
    // Initialize sensor
    if (!sensor_task_init()) {
        Serial.println("[ERROR] Sensor initialization failed!");
        vTaskDelete(NULL);
        return;
    }
    
    CE_SensorData sensor_data, sensor_data_prev = {0};
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t read_count = 0;
    
    while (1) {
        // Wait for next cycle
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_READ_INTERVAL));
        
        // Read DHT20
        int result = dht20_ptr->read();
        
        if (result != DHT20_OK) {
            Serial.printf("[SENSOR] Read error: %d\n", result);
            sensor_state.error_count++;
            continue;
        }
        
        float temp_raw = dht20_ptr->getTemperature();
        float hum_raw = dht20_ptr->getHumidity();
        
        // Validate
        if (!sensor_validate(temp_raw, hum_raw)) {
            Serial.printf("[SENSOR] Invalid reading: T=%.1f, H=%.1f\n", temp_raw, hum_raw);
            sensor_state.error_count++;
            continue;
        }
        
        // Store for outlier detection
        sensor_state.temp_readings[sensor_state.read_index] = temp_raw;
        sensor_state.hum_readings[sensor_state.read_index] = hum_raw;
        
        // Calculate rolling statistics
        float temp_mean = 0.0f, hum_mean = 0.0f;
        for (int i = 0; i < FILTER_OUTLIER_WINDOW; i++) {
            temp_mean += sensor_state.temp_readings[i];
            hum_mean += sensor_state.hum_readings[i];
        }
        temp_mean /= FILTER_OUTLIER_WINDOW;
        hum_mean /= FILTER_OUTLIER_WINDOW;
        
        // Calculate standard deviation
        float temp_var = 0.0f, hum_var = 0.0f;
        for (int i = 0; i < FILTER_OUTLIER_WINDOW; i++) {
            temp_var += (sensor_state.temp_readings[i] - temp_mean) *
                        (sensor_state.temp_readings[i] - temp_mean);
            hum_var += (sensor_state.hum_readings[i] - hum_mean) *
                       (sensor_state.hum_readings[i] - hum_mean);
        }
        temp_var /= FILTER_OUTLIER_WINDOW;
        hum_var /= FILTER_OUTLIER_WINDOW;
        
        float temp_stddev = sqrt(temp_var);
        float hum_stddev = sqrt(hum_var);
        
        // Check for outliers
        bool temp_outlier = sensor_is_outlier(temp_raw, temp_mean, temp_stddev,
                                              FILTER_ZSCORE_THRESHOLD);
        bool hum_outlier = sensor_is_outlier(hum_raw, hum_mean, hum_stddev,
                                             FILTER_ZSCORE_THRESHOLD);
        
        if (temp_outlier || hum_outlier) {
            // Use previous values instead
            temp_raw = sensor_state.temp_smoothed;
            hum_raw = sensor_state.hum_smoothed;
            Serial.printf("[SENSOR] Outlier detected, using smoothed values\n");
        }
        
        // Apply exponential smoothing
        sensor_state.temp_smoothed = sensor_smooth(FILTER_ALPHA, temp_raw,
                                                   sensor_state.temp_smoothed);
        sensor_state.hum_smoothed = sensor_smooth(FILTER_ALPHA, hum_raw,
                                                  sensor_state.hum_smoothed);
        
        // Update global state
        g_lastTemperature = sensor_state.temp_smoothed;
        g_lastHumidity = sensor_state.hum_smoothed;
        g_sensorValid = true;
        
        // Prepare queue message
        sensor_data.temperature = sensor_state.temp_smoothed;
        sensor_data.humidity = sensor_state.hum_smoothed;
        sensor_data.timestamp = time(NULL);
        sensor_data.valid = true;
        sensor_data.anomalyScore = 0.0f;  // Will be set by TinyML task
        sensor_data.anomaly = false;
        sensor_data.dataQuality = sensor_calc_quality();
        
        // Send to queue
        if (xQueueSend(xQueueSensorData, &sensor_data, pdMS_TO_TICKS(100)) != pdTRUE) {
            Serial.println("[SENSOR] Queue send timeout");
        } else {
            if (++read_count % 10 == 0) {  // Log every 10th reading
                Serial.printf("[SENSOR] T=%.2f°C, H=%.2f%%, Q=%.2f, #%lu\n",
                            sensor_data.temperature, sensor_data.humidity,
                            sensor_data.dataQuality, read_count);
            }
        }
        
        // Advance circular buffer index
        sensor_state.read_index = (sensor_state.read_index + 1) % FILTER_OUTLIER_WINDOW;
    }
}
