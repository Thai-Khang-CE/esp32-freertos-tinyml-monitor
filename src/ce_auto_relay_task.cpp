#include "ce_auto_relay_task.h"
#include "ce_config_global.h"
#include <Arduino.h>

// ============================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================

/**
 * @brief Set relay state with debounce
 */
static void relay_set(uint8_t pin, bool state) {
    digitalWrite(pin, state ? RELAY_ON_STATE : RELAY_OFF_STATE);
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Debounce delay
}

/**
 * @brief Get current light level from ADC
 * @return Light value (0-4095 for 12-bit ADC)
 */
static uint16_t read_light_value(void) {
    return analogRead(LIGHT_PIN);
}

/**
 * @brief Check if human is inside (PIR sensor)
 * @return true if PIR detects motion (human), false otherwise
 */
static bool is_human_inside(void) {
    return digitalRead(PIR_PIN) == HIGH;
}

/**
 * @brief Initialize relay pins and sensors
 */
static void relay_auto_init(void) {
    // Configure relay outputs
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
    pinMode(RELAY_FAN_PIN, OUTPUT);
    
    // Configure sensor inputs
    pinMode(PIR_PIN, INPUT);
    pinMode(LIGHT_PIN, INPUT);
    
    // Set initial state: all OFF
    relay_set(RELAY_LIGHT_PIN, false);
    relay_set(RELAY_FAN_PIN, false);
    g_lightOn = false;
    g_fanOn = false;
    
    Serial.println("[AUTO_RELAY] Initialized");
}

/**
 * @brief Control fan based on temperature and human presence
 * @param temperature Current temperature reading
 * @param human_inside Whether human is inside
 */
static void control_fan_auto(float temperature, bool human_inside) {
    bool should_fan_be_on = false;
    
    // Fan ON conditions:
    // 1. Temperature > 30°C AND
    // 2. Human inside
    if (temperature > 30.0f && human_inside) {
        should_fan_be_on = true;
    } else {
        should_fan_be_on = false;
    }
    
    // If state changed, update relay
    if (should_fan_be_on != g_fanOn) {
        relay_set(RELAY_FAN_PIN, should_fan_be_on);
        g_fanOn = should_fan_be_on;
        
        Serial.printf("[AUTO_RELAY] FAN: %s (temp=%.1f°C, human=%s)\n",
                      g_fanOn ? "ON" : "OFF", temperature, human_inside ? "YES" : "NO");
    }
}

/**
 * @brief Control light based on human presence and light level
 * @param light_level Current light sensor reading (0-4095)
 * @param human_inside Whether human is inside
 */
static void control_light_auto(uint16_t light_level, bool human_inside) {
    // Light threshold: turn on light when light level is below threshold
    // Assuming light sensor reads: HIGH value = BRIGHT, LOW value = DARK
    // Threshold of ~1500 means turn light on when below ~1500 (dark room)
    const uint16_t LIGHT_THRESHOLD = 1500;
    
    bool should_light_be_on = false;
    
    // Light ON conditions:
    // 1. Human inside AND
    // 2. Light level is below threshold (dark room)
    if (human_inside && light_level < LIGHT_THRESHOLD) {
        should_light_be_on = true;
    } else {
        should_light_be_on = false;
    }
    
    // If state changed, update relay
    if (should_light_be_on != g_lightOn) {
        relay_set(RELAY_LIGHT_PIN, should_light_be_on);
        g_lightOn = should_light_be_on;
        
        Serial.printf("[AUTO_RELAY] LIGHT: %s (light_level=%u, human=%s)\n",
                      g_lightOn ? "ON" : "OFF", light_level, human_inside ? "YES" : "NO");
    }
}

// ============================================================
// MAIN TASK FUNCTION
// ============================================================

/**
 * @brief Main auto relay task
 * 
 * Receives sensor data from queue (temp_humi_monitor task sends data)
 * Automatically controls relays based on:
 * - Temperature (fan control)
 * - Light level + human presence (light control)
 * - Auto-mode flags (IsLight_Auto, IsFan_Auto)
 * 
 * @param parameter FreeRTOS task parameter (unused)
 */
void ce_auto_relay_task(void *parameter) {
    SensorData sensor_reading;
    uint32_t last_log_time = 0;
    const uint32_t LOG_INTERVAL_MS = 10000;  // Log every 10 seconds
    
    // Initialize relay pins
    relay_auto_init();
    
    Serial.println("[AUTO_RELAY] Task started - waiting for sensor data...");
    
    while (1) {
        // Try to receive sensor data from queue (5 second timeout)
        if (xQueueReceive(xQueueTempHumiForMain, &sensor_reading, 
                          5000 / portTICK_PERIOD_MS) == pdTRUE) {
            
            // Control fan if auto mode is enabled
            if (IsFan_Auto) {
                control_fan_auto(sensor_reading.temperature, sensor_reading.human_inside);
            }
            
            // Control light if auto mode is enabled
            if (IsLight_Auto) {
                control_light_auto((uint16_t)sensor_reading.light, sensor_reading.human_inside);
            }
            
            // Periodic logging (every 10 seconds)
            uint32_t now = millis();
            if (now - last_log_time >= LOG_INTERVAL_MS) {
                last_log_time = now;
                Serial.printf("[AUTO_RELAY] STATUS: Light=%s, Fan=%s | "
                              "Temp=%.1f°C, Light=%u, Human=%s | "
                              "Auto(Light=%s, Fan=%s)\n",
                              g_lightOn ? "ON" : "OFF", g_fanOn ? "ON" : "OFF",
                              sensor_reading.temperature, (uint16_t)sensor_reading.light,
                              sensor_reading.human_inside ? "YES" : "NO",
                              IsLight_Auto ? "ON" : "OFF", IsFan_Auto ? "ON" : "OFF");
            }
            
        } else {
            // Timeout: no sensor data received
            // This could mean temp_humi_monitor task is not running or queue is not sending
            Serial.println("[AUTO_RELAY] TIMEOUT: No sensor data in queue");
            
            // Optional: Turn off relays on timeout (safety feature)
            // relay_set(RELAY_LIGHT_PIN, false);
            // relay_set(RELAY_FAN_PIN, false);
            // g_lightOn = false;
            // g_fanOn = false;
        }
    }
}