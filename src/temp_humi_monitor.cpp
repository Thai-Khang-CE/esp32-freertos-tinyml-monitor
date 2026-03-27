#include "temp_humi_monitor.h"
#include "ce_config_global.h"
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);


void temp_humi_monitor(void *pvParameters){
    pinMode(PIR_PIN, INPUT);
    pinMode(LIGHT_PIN, INPUT);

    Wire.begin(11, 12);
    dht20.begin();

    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Task B-");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");

    
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();
        float read_light = analogRead(LIGHT_PIN);
        bool human_inside = digitalRead(PIR_PIN) == HIGH;
        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sensor Error!");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            //return;
        } else
        
        {
        SensorData sensordata;
        sensordata.temperature = temperature;
        sensordata.humidity = humidity;
        sensordata.light = read_light;
        sensordata.human_inside = human_inside;
        
       
        xQueueSend(xQueueForTinyML, &sensordata, 0);
        xQueueSend(xQueueTempHumiForMain, &sensordata, 0); 


            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("Temp: ");
            lcd.print(temperature, 1); 
            lcd.print((char)223);
            lcd.print("C");

            lcd.setCursor(0, 1);
            lcd.print("Humi: ");
            lcd.print(humidity, 1); 
            lcd.print("%");

            lcd.setCursor(11, 0); 
            if (temperature >= 35.0) { 
                lcd.print("CRIT!");
            } else if (temperature >= 30.0) {
                lcd.print("WARN ");
            } else { 
                lcd.print("NORM ");
            }

            vTaskDelay(2500 / portTICK_PERIOD_MS);

            // --- MÀN HÌNH 2: HIỂN THỊ PIR & ÁNH SÁNG ---
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PIR: ");
            if (human_inside) { // Có người
                lcd.print("Motion!");
            } else {                // Không có người
                lcd.print("Clear  ");
            }

            lcd.setCursor(0, 1);
            lcd.print("Light: ");
            lcd.print(read_light);

            // Dừng 2.5 giây trước khi quay lại màn hình 1
            vTaskDelay(2500 / portTICK_PERIOD_MS);
        }

        // Print the results
        vTaskDelay(5000);
    }
}