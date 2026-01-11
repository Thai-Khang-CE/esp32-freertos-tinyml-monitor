#include "led_blinky.h"
#include "global.h"
void led_blinky(void *pvParameters)
{
  pinMode(LED_GPIO, OUTPUT);
  TickType_t xfrequency = pdMS_TO_TICKS(250);
  float temperature = 0;
  SensorData data_receive;
  while (1)
  {
    if (xQueueReceive(xQueueForLedBlink, &data_receive, portMAX_DELAY) == pdPASS)
    {
      temperature = data_receive.temperature;

      if (temperature < (float)30)
        xfrequency = pdMS_TO_TICKS(500);
      else if (temperature >= (float)30 && temperature <= (float)35)
        xfrequency = pdMS_TO_TICKS(250);
      else
        xfrequency = pdMS_TO_TICKS(125);
    
    if(xSemaphoreTake(xSemaphoreMutex, portMAX_DELAY) == pdTRUE)
    {
      Serial.print("[LED] Temperature: ");
      Serial.print(temperature, 2);
      Serial.print("°C | Frequency: ");
      Serial.print(1.0 / (((float)xfrequency * 2) / 1000.0), 1);
      Serial.println(" HZ");

      xSemaphoreGive(xSemaphoreMutex);
    }
    }
    digitalWrite(LED_GPIO, HIGH);
    vTaskDelay(xfrequency);
    digitalWrite(LED_GPIO, LOW);
    vTaskDelay(xfrequency);
  }
}
