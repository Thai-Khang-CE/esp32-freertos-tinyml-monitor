#ifndef CE_AUTO_RELAY_TASK_H
#define CE_AUTO_RELAY_TASK_H

#include <Arduino.h>
#include "ce_config_global.h"
#include "ce_data_types.h"

/*============================================================
  SMART HOME CE AUTO RELAY TASK
  
  Purpose: Automatically control relays based on sensor data
  
  Logic:
    - FAN Auto: ON when temp > 30°C AND human inside
               OFF when no human
    - LIGHT Auto: ON when human inside AND light level < threshold
                 OFF when no human
  
  Queue: Receives SensorData from xQueueTempHumiForMain
  External Control: IsLight_Auto, IsFan_Auto flags to enable/disable modes
  
  Author: Thai Khang & Dat
  ============================================================*/

// ============================================================
// MAIN TASK FUNCTION
// ============================================================

/**
 * @brief FreeRTOS task for automatic relay control
 * Receives temperature/humidity from queue and controls relays accordingly
 * @param parameter FreeRTOS task parameter (unused)
 */
void ce_auto_relay_task(void *parameter);

#endif


