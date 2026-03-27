#ifndef CE_AUTO_RELAY_TASK_C
#define CE_AUTO_RELAY_TASK_C

#include "ce_config_global.h"
#include "ce_data_types.h"

/*
@brief Task to automatically control relays
@Logic: 
      Fanauto: on when temp > 30C and human inside
               off when no human
      Lightauto: on when human inside and base on light sensor 
                off when no human    
@return: void
@author : Thai Khang and Dat     
*/
void ce_auto_relay_task(void *parameter);


#endif


