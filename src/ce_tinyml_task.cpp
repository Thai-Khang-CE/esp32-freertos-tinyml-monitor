#include "ce_tinyml_task.h"
#include "ce_global.h"
#include "ce_config.h"

void tinyml_task(void *parameter) {
    Serial.println("[TINYML] Task started (stub)");
    
    // Placeholder: TODO - Integrate real TensorFlow Lite model
    // For now, simple threshold-based anomaly detection
    
    CE_SensorData sensor = {0};
    
    while (1) {
        if (xQueueReceive(xQueueSensorData, &sensor, pdMS_TO_TICKS(5000))) {
            // Normalize inputs (0 to 1 range)
            float temp_norm = (sensor.temperature + 40.0f) / 125.0f;  // -40 to +85°C → 0 to 1
            float hum_norm = sensor.humidity / 100.0f;                  // 0 to 100% → 0 to 1
            
            // Clamp
            if (temp_norm < 0.0f) temp_norm = 0.0f;
            if (temp_norm > 1.0f) temp_norm = 1.0f;
            if (hum_norm < 0.0f) hum_norm = 0.0f;
            if (hum_norm > 1.0f) hum_norm = 1.0f;
            
            // Simple anomaly score: deviation from nominal range
            float nominal_temp_center = 0.45f;  // 25°C center
            float nominal_hum_center = 0.5f;
            
            float temp_dev = fabs(temp_norm - nominal_temp_center);
            float hum_dev = fabs(hum_norm - nominal_hum_center);
            float anomaly_score = (temp_dev + hum_dev) / 2.0f;
            
            // Clamp score to [0, 1]
            if (anomaly_score > 1.0f) anomaly_score = 1.0f;
            
            sensor.anomalyScore = anomaly_score;
            sensor.anomaly = (anomaly_score > TINYML_ANOMALY_THRESHOLD);
            
            // Update globals
            g_lastAnomalyScore = anomaly_score;
            
            // Re-send to upload queue
            xQueueSend(xQueueSensorData, &sensor, pdMS_TO_TICKS(100));
            
            Serial.printf("[TINYML] Score: %.2f, Anomaly: %d\n", anomaly_score, sensor.anomaly);
        }
    }
}
