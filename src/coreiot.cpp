#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "jl4q7d3opj79wn9xdol9";   // Device Access Token
const int   mqttPort = 1883;
static unsigned int current_control_led = 11;
static unsigned int initialize = 0;
SensorData sensorData; 

// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

void setupPin()
{
  current_mode = 1;
  pinMode(LED_IOT_1,OUTPUT);
  pinMode(LED_IOT_2,OUTPUT);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) 
{
  // ledcDetachPin(LED_IOT_1);
  // ledcDetachPin(LED_IOT_2);
  // digitalWrite(LED_IOT_1,LOW);
  // digitalWrite(LED_IOT_2,LOW);
  Serial.print("Message arrived [");
  Serial.print(topic);   // dia chi duong dan cua mqtt tra ve
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);   // payload la message ma mqtt broker gui ve
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) 
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  //const char* params = doc["params"];
  bool is_on = doc["params"].as<bool>();
  if(strcmp(method,"setValueLED1") == 0)
     current_control_led = LED_IOT_1;
  else if(strcmp(method,"setValueLED2") == 0)
    current_control_led = LED_IOT_2;
  else 
  {
      Serial.println("Unknow method");
      return;
  }
  
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
   // const char* params = doc["params"];

    if (is_on && current_control_led != 11) 
    {
      Serial.println("Device turned ON.");
      digitalWrite(current_control_led,HIGH);
    } 
    else if(current_control_led != 11)
    {
      Serial.println("Device turned OFF.");
      digitalWrite(current_control_led,LOW);
    }
    StaticJsonDocument<100> response_doc;
    response_doc["value"] = is_on; // Tự động xử lý boolean

    String json_response;
    serializeJson(response_doc, json_response); // Chuyển thành chuỗi {"value":true} hoặc {"value":false}

    Serial.print("Publishing to attributes: ");
    Serial.println(json_response);  
    client.publish("v1/devices/me/attributes", json_response.c_str());
  
 
}


void setup_coreiot(){

  //Serial.print("Connecting to WiFi...");
  //WiFi.begin(wifi_ssid, wifi_password);
  //while (WiFi.status() != WL_CONNECTED) {
  
  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){

    setup_coreiot();
    setupPin();

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Sample payload, publish to 'v1/devices/me/telemetry'

        // Kiểm tra xem đã đến lúc gửi telemetry (5 giây)
        if (xQueueReceive(xQueueForIoT, &sensorData, 0) == pdTRUE)
        {
             // Cập nhật thời gian publish cuối

            // Lấy giá trị cảm biến
            // LƯU Ý: Nếu glob_temperature và glob_humidity được ghi bởi 1 TASK KHÁC,
            // bạn PHẢI dùng Mutex để bảo vệ chúng khi đọc/ghi.
            float temperature = sensorData.temperature;
            float humidity = sensorData.humidity;
          
            String payload = "{\"temperature\":" + String(temperature) +  ",\"humidity\":" + String(humidity) + "}";
            
            client.publish("v1/devices/me/telemetry", payload.c_str());

            Serial.println("Published payload: " + payload);
        }
        vTaskDelay(10);  // Publish every 10 seconds
    }
}