#ifndef CE_SENSOR_TASK_H
#define CE_SENSOR_TASK_H

#include "ce_data_types.h"
#include "ce_config.h"

/*============================================================
  SENSOR TASK - DHT20 TEMPERATURE/HUMIDITY READING
  ============================================================*/

/**
 * @brief Initialize DHT20 sensor
 * @return true if successful, false otherwise
 */
bool sensor_task_init(void);

/**
 * @brief FreeRTOS task for sensor reading
 * Runs every SENSOR_READ_INTERVAL ms
 * Reads DHT20, applies filtering, sends to xQueueSensorData
 */
void sensor_task(void *parameter);

/**
 * @brief Validate sensor reading (range check)
 * @param temp Temperature in °C
 * @param hum Humidity in %
 * @return true if valid, false otherwise
 */
bool sensor_validate(float temp, float hum);

/**
 * @brief Apply exponential smoothing filter
 * @param alpha Smoothing factor (0.0-1.0), typically 0.3
 * @param raw Raw value
 * @param smoothed Previous smoothed value
 * @return New smoothed value
 */
float sensor_smooth(float alpha, float raw, float smoothed);

/**
 * @brief Detect and remove outliers using Z-score
 * @param value Current value
 * @param mean Average of recent readings
 * @param stddev Standard deviation
 * @param threshold Z-score threshold (typically 2.5)
 * @return true if value is outlier, false otherwise
 */
bool sensor_is_outlier(float value, float mean, float stddev, float threshold);

/**
 * @brief Calculate data quality metric (0.0-1.0)
 * Based on reading stability and error count
 * @return Quality score
 */
float sensor_calc_quality(void);

#endif // CE_SENSOR_TASK_H
