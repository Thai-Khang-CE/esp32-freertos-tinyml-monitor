#include "global.h"

#include "led_blinky.h"
#include "temp_humi_monitor.h"
//#include "mainserver.h"
#include "tinyml.h"


// include task
#include "task_toogle_boot.h"
#include "mainserver.h"

// CE Firmware includes
#include "ce_global.h"
#include "ce_config.h"
#include "ce_relay_task.h"
#include "ce_http_upload_task.h"
#include "ce_command_poll_task.h"
#include "ce_wifi_manager.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("[BOOT] Serial initialized at 115200");
  
  // Initialize CE globals (queues, semaphores)
  ce_globals_init();
  Serial.println("[BOOT] CE globals initialized");
  
  // Create legacy tasks (old project)
  xTaskCreate(led_blinky, "Task LED Blink", 4096, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 4096, NULL, 2, NULL);
  xTaskCreate(main_server_task, "Task Main Server" , 16384  ,NULL  ,2 , NULL);
  xTaskCreate(tiny_ml_task, "Tiny ML Task" ,8192  ,NULL  ,2 , NULL);
 
  
  // Create CE tasks
  Serial.println("[BOOT] Creating CE tasks...");
  
  xTaskCreatePinnedToCore(wifi_manager_task, "WiFi Manager", WIFI_TASK_STACK, NULL, WIFI_TASK_PRIO, NULL, 1);
  xTaskCreatePinnedToCore(relay_task, "Relay Control", RELAY_TASK_STACK, NULL, RELAY_TASK_PRIO, NULL, 0);
  xTaskCreatePinnedToCore(http_upload_task, "HTTP Upload", HTTP_UPLOAD_TASK_STACK, NULL, HTTP_UPLOAD_TASK_PRIO, NULL, 0);
  xTaskCreatePinnedToCore(command_poll_task, "Command Poll", HTTP_COMMAND_TASK_STACK, NULL, HTTP_COMMAND_TASK_PRIO, NULL, 0);
  
  Serial.println("[BOOT] Setup complete - all tasks created");
}

void loop()
{
  // if (check_info_File(1))
  // {
  //   if (!Wifi_reconnect())
  //   {
  //     Webserver_stop();
  //   }
  //   else
  //   {
  //     CORE_IOT_reconnect();
  //   }
  // }
  // Webserver_reconnect();
}