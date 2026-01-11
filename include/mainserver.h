#ifndef ___MAIN_SERVER__
#define ___MAIN_SERVER__
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "global.h"
#include "Adafruit_NeoPixel.h"
//#include "task_wifi.h"

#define LED1_PIN 47
#define LED2_PIN 5

#define BOOT_PIN 0

extern WebServer server;

extern bool isAPMode;

extern int current_mode;



String mainPage();
String settingsPage();

void startAP();
void setupServer();
void connectToWiFi();

void main_server_task(void *pvParameters);

#endif