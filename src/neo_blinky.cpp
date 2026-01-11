#include "neo_blinky.h"
#include "global.h"

void neo_blinky(void *pvParameters)
{

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();
    SensorData data_receive;

    while (1)
    {

        if (xQueueReceive(xQueueForNeoPixel, &data_receive, portMAX_DELAY) == pdTRUE)
        {
            float humidity = data_receive.humidity;

            uint32_t color;
            String status;
            String colorName; // Biến lưu tên màu để in log

            // --- LOGIC XÁC ĐỊNH MÀU SẮC ---
            if (humidity < 0 || isnan(humidity))
            {
                color = strip.Color(255, 255, 255); // Trắng
                status = "ERROR";
                colorName = "White";
            }
            else if (humidity >= 0 && humidity < 60)
            {
                color = strip.Color(255, 0, 0); // Đỏ
                status = "LOW";
                colorName = "Red";
            }
            else if (humidity >= 60 && humidity < 80)
            {
                color = strip.Color(0, 255, 0); // Xanh lá
                status = "NORMAL";
                colorName = "Green";
            }
            else // humidity >= 80
            {
                color = strip.Color(0, 0, 255); // Xanh dương
                status = "WET";
                colorName = "Blue";
            }

            // Cập nhật đèn LED vật lý
            strip.setPixelColor(0, color);
            strip.show();

            // --- IN LOG (ĐƯỢC BẢO VỆ BỞI MUTEX) ---
            // Kiểm tra xem có lấy được chìa khóa Mutex không
            if (xSemaphoreTake(xSemaphoreMutex, portMAX_DELAY) == pdTRUE)
            {
                // Tạo một chuỗi duy nhất chứa toàn bộ thông tin cần in
                // Việc này giúp in 1 lần atomic, tránh bị ngắt quãng giữa chừng
                String logMessage = "[NeoPixel] Update - Humidity: " + String(humidity, 1) +
                                    "% | Status: " + status +
                                    " | Color: " + colorName;

                Serial.println(logMessage); // In 1 dòng duy nhất

                // Trả chìa khóa ngay lập tức sau khi in xong
                xSemaphoreGive(xSemaphoreMutex);
            }
        }

        // Delay nhỏ để tránh spam CPU nếu dữ liệu đến quá dồn dập
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
