#ifndef __COREIOT_H__
#define __COREIOT_H__

#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mainserver.h"

#define LED_IOT_1 47
#define LED_IOT_2 5
void coreiot_task(void *pvParameters);

#endif