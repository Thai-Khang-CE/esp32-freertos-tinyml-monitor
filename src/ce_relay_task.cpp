#include "ce_relay_task.h"
#include "ce_config_global.h"

bool relay_task_init(void) {
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
    pinMode(RELAY_FAN_PIN, OUTPUT);
    digitalWrite(RELAY_LIGHT_PIN, RELAY_OFF_STATE);
    digitalWrite(RELAY_FAN_PIN, RELAY_OFF_STATE);
    g_lightOn = false;
    g_fanOn = false;
    Serial.println("[RELAY] Init OK");
    return true;
}

bool relay_set(RelayDevice device, bool state) {
    uint8_t pin = (device == RELAY_DEVICE_LIGHT) ? RELAY_LIGHT_PIN : RELAY_FAN_PIN;
    uint8_t level = state ? RELAY_ON_STATE : RELAY_OFF_STATE;
    
    digitalWrite(pin, level);
    vTaskDelay(pdMS_TO_TICKS(100)); // Short delay to allow relay to switch
    
    // Verify
    uint8_t read_level = digitalRead(pin);
    bool success = (read_level == level);
    
    if (device == RELAY_DEVICE_LIGHT) {
        g_lightOn = state;
    } else {
        g_fanOn = state;
    }
    
    Serial.printf("[RELAY] %s=%d (verify=%d)\n", 
                  device == RELAY_DEVICE_LIGHT ? "LIGHT" : "FAN", state, success);
    return success;
}

bool relay_get(RelayDevice device) {
    return (device == RELAY_DEVICE_LIGHT) ? g_lightOn : g_fanOn;
}

void relay_task(void *parameter) {
    Serial.println("[RELAY] Task started");
    if (!relay_task_init()) {
        vTaskDelete(NULL);
        return;
    }
    
    Command cmd = {0};
    DeviceStatusReport status = {0};
    
    while (1) {
        if (xQueueReceive(xQueueCommand, &cmd, pdMS_TO_TICKS(1000))) {
            Serial.printf("[RELAY] Executing: %s=%d\n", 
                         cmd.device == RELAY_DEVICE_LIGHT ? "LIGHT" : "FAN", cmd.action);
            
            bool success = relay_set((RelayDevice)cmd.device, (bool)cmd.action);
            
            // Report status
            status.status.lightOn = g_lightOn;
            status.status.fanOn = g_fanOn;
            status.status.timestamp = time(NULL);
            strncpy(status.status.lastCommandId, cmd.commandId, sizeof(status.status.lastCommandId)-1);
            status.status.executionSuccess = success;
            
            xQueueSend(xQueueStatus, &status, pdMS_TO_TICKS(100));
        }
    }
}
