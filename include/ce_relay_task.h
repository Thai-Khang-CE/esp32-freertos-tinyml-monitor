#ifndef CE_RELAY_TASK_H
#define CE_RELAY_TASK_H

#include "ce_data_types.h"

/**
 * @brief Initialize relay GPIO pins
 */
bool relay_task_init(void);

/**
 * @brief FreeRTOS task for relay control
 * Receives commands from xQueueCommand, executes relays, sends status
 */
void relay_task(void *parameter);

/**
 * @brief Set relay state (ON/OFF)
 * @param device Light or Fan
 * @param state true=ON, false=OFF
 * @return true if successful
 */
bool relay_set(RelayDevice device, bool state);

/**
 * @brief Get current relay state
 */
bool relay_get(RelayDevice device);

#endif
