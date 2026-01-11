#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>
#include "global.h"


bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;
int current_mode = 1;
static unsigned long lastBlink1 = 0;
static unsigned long lastBlink2 = 0;


WebServer server(80);

unsigned long connect_start_ms = 0;
bool connecting = false;
void setupPinMode()
{
 
  pinMode(LED1_PIN,OUTPUT);
  pinMode(LED2_PIN,OUTPUT);
}

void PWMSetUP()
{
  ledcSetup(0, 1000, 8);
  ledcAttachPin(LED1_PIN, 0);

  // LED2 dùng channel 1
  ledcSetup(1, 1000, 8);
  ledcAttachPin(LED2_PIN, 1);
}

void PWMdetach()
{
  ledcDetachPin(LED1_PIN);
  ledcDetachPin(LED2_PIN);
}
String mainPage()
{
return  R"rawliteral(
   <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Smart Dashboard</title>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background: linear-gradient(135deg, #6a11cb, #2575fc);
      color: #222;
      text-align: center;
      margin: 0;
      padding: 0;
    }

    .container {
      margin: 60px auto;
      max-width: 700px; /* 🟢 Tăng từ 420px → 700px */
      background: #ffffffee;
      border-radius: 25px;
      box-shadow: 0 12px 30px rgba(0,0,0,0.25);
      padding: 50px 60px; /* 🟢 Tăng padding */
      backdrop-filter: blur(10px);
      transition: transform 0.3s ease, box-shadow 0.3s;
    }

    .container:hover {
      transform: translateY(-6px);
      box-shadow: 0 14px 35px rgba(0,0,0,0.3);
    }

    h1 {
      color: #333;
      font-size: 36px; /* 🟢 To hơn */
      margin-bottom: 40px;
    }

    .sensor-card {
      display: flex;
      justify-content: space-between;
      align-items: center;
      background: #f5f7ff;
      padding: 20px 30px; /* 🟢 To hơn */
      margin: 18px 0;
      border-radius: 16px;
      box-shadow: inset 0 0 10px rgba(0,0,0,0.05);
      font-size: 22px;
    }

    .sensor-label {
      font-weight: 600;
      color: #333;
    }

    .sensor-value {
      color: #1b5e20;
      font-weight: bold;
      font-size: 24px;
    }
    
    .score-value {
      display: block;
      font-size : 14px;
      color: #666;
      font-weight: normal;
      margin-top: 4px;
      text-align: right;
    }

    .led-panel {
      margin-top: 40px;
    }

    .led-button {
      display: inline-block;
      width: 220px; /* 🟢 Tăng chiều rộng */
      padding: 18px 0; /* 🟢 Tăng chiều cao */
      margin: 15px;
      border: none;
      border-radius: 14px;
      font-size: 20px;
      font-weight: 500;
      color: white;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 6px 12px rgba(0,0,0,0.25);
    }

    .on {
      background: linear-gradient(135deg, #00e676, #00c853);
    }

    .off {
      background: linear-gradient(135deg, #ff5252, #d50000);
    }

    .led-button:hover {
      transform: scale(1.07);
      box-shadow: 0 10px 20px rgba(0,0,0,0.25);
    }

    #settings {
      position: absolute;
      top: 20px;
      right: 20px;
      background: #007bff;
      color: white;
      border: none;
      padding: 10px 16px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 18px;
      transition: 0.3s;
    }

    #settings:hover {
      background: #0056b3;
    }

    footer {
      margin-top: 30px;
      color: #eee;
      font-size: 16px;
      text-shadow: 0 1px 2px rgba(0,0,0,0.2);
    }

    .mode-panel {
      margin-bottom: 25px;
    }

    .mode-button {
      margin: 10px;
      padding: 14px 20px;
      border: none;
      border-radius: 10px;
      font-size: 18px;
      cursor: pointer;
      background: #e0e0e0;
      transition: 0.3s;
    }
    .mode-button.active {
      background: linear-gradient(135deg, #6a11cb, #2575fc);
      color: white;
    }
    .mode-button:hover {
      transform: scale(1.05);
    }
    
    .led-panel {
      display: none; /* Ẩn mặc định */
    }

    .led-panel.show {
      display: block; /* Hiện khi có class 'show' */
    }
  </style>
</head>
<body>
  <button id="settings" onclick="window.location='/settings'">&#9881; Settings</button>

  <div class="container">
    <h1>🌡️ ESP32 Smart Dashboard</h1>

    <div class="sensor-card">
      <div class="sensor-label">Temperature</div>
      <div class="sensor-value"><span id="temp">loading...</span> °C</div>
    </div>

    <div class="sensor-card">
      <div class="sensor-label">Humidity</div>
      <div class="sensor-value"><span id="hum">loading...</span> %</div>
    </div>  
   
    <div class="sensor-card">
      <div class="sensor-label">AI Diagnosis</div>
      <div class="sensor-value">
          <span id="anom_type">WAITING</span>
          <span class="score-value">(Score: <span id="anom_score">0.00</span>)</span>
      </div>
    </div>
    <div class="mode-panel">
      <button class="mode-button active" onclick="setMode(1)">🕹️ Mode 1: Manual</button>
      <button class="mode-button" onclick="setMode(2)">⚡ Mode 2: Blink</button>
      <button class="mode-button" onclick="setMode(3)">🌡️ Mode 3: PWM Temp</button>
    </div>
    
    <div id="ledPanel" class="led-panel">
      <button id="led1Btn" class="led-button off" onclick="toggleLED(1)">💡 LED 1: OFF</button>
      <button id="led2Btn" class="led-button off" onclick="toggleLED(2)">💡 LED 2: OFF</button>
    </div>
  </div>

  <footer>© 2025 ESP32 IoT Dashboard</footer>

  <script>
    function toggleLED(id) {
      fetch(`/toggle?led=${id}`)
        .then(res => res.json())
        .then(data => {
          updateLED('led1Btn', data.led1);
          updateLED('led2Btn', data.led2);
        })
        .catch(err => console.error(err));
    }

    function updateLED(buttonId, state) {
      const btn = document.getElementById(buttonId);
      const ledNum = buttonId.includes('1') ? 1 : 2;

      if (state === 'ON') {
        btn.className = 'led-button on';
        btn.innerText = `💡 LED ${ledNum}: ON`;
      } else {
        btn.className = 'led-button off';
        btn.innerText = `💡 LED ${ledNum}: OFF`;
      }
    }

    function setMode(mode) {
      fetch(`/mode?set=${mode}`)
        .then(res => res.json())
        .then(data => {
          mode = Number(mode);
          const ledPanel = document.getElementById('ledPanel');
          
           if (mode === 1) {
            ledPanel.classList.add('show'); // Hiện đèn khi mode 1
          } else {
            ledPanel.classList.remove('show'); // Ẩn khi mode 2 hoặc 3
          }

          const buttons = document.querySelectorAll('.mode-button');
          buttons.forEach(b => b.classList.remove('active'));
          buttons[mode - 1].classList.add('active');
          console.log("Mode changed:", data.mode);

        })
        .catch(err => console.error(err));
    }

    setInterval(() => {
      fetch('/sensors')
        .then(res => res.json())
        .then(data => {
          document.getElementById('temp').innerText = data.temp;
          document.getElementById('hum').innerText = data.hum;

          const anomTypeEl = document.getElementById('anom_type');
          const anomScoreEl = document.getElementById('anom_score');

          if (anomTypeEl && anomScoreEl) {
              anomTypeEl.innerText = data.anom_type;
              anomScoreEl.innerText = data.anom_score;
              
              if (data.anom_type === 'CRITICAL') {
                 anomTypeEl.style.color = '#d50000'; // Đỏ đậm
              } else if (data.anom_type === 'WARNING') {
                 anomTypeEl.style.color = '#ff9800'; // Cam
              } else {
                 anomTypeEl.style.color = '#1b5e20'; // Xanh lá
              }
          }
        })
        .catch(err => console.error(err));
    }, 2000); // 2 giây cập nhật 1 lần
  </script>
</body>
</html>
)rawliteral";
}



String settingsPage() 
{
  return R"rawliteral(
    <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wi-Fi Settings</title>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background: linear-gradient(135deg, #6a11cb, #2575fc);
      color: #222;
      text-align: center;
      margin: 0;
      padding: 0;
    }

    .container {
      margin: 60px auto;
      max-width: 500px; /* Nhỏ hơn mainPage một chút */
      background: #ffffffee;
      border-radius: 25px;
      box-shadow: 0 12px 30px rgba(0,0,0,0.25);
      padding: 40px 50px;
      backdrop-filter: blur(10px);
    }

    h2 {
      color: #333;
      font-size: 32px;
      margin-bottom: 30px;
    }

    /* Selector này giờ sẽ hoạt động cho cả hai ô */
    input[type=text],
    input[type=password] {
      width: calc(100% - 24px); /* 100% trừ padding */
      padding: 14px 12px;
      margin-bottom: 20px;
      border: 1px solid #ccc;
      border-radius: 10px;
      font-size: 18px;
      font-family: 'Segoe UI', sans-serif;
      box-shadow: inset 0 2px 4px rgba(0,0,0,0.05);
    }

    button {
      width: 160px;
      padding: 14px 0;
      margin: 10px;
      border: none;
      border-radius: 12px;
      font-size: 18px;
      font-weight: 500;
      color: white;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 6px 12px rgba(0,0,0,0.2);
    }

    button:hover {
      transform: scale(1.07);
      box-shadow: 0 8px 16px rgba(0,0,0,0.25);
    }

    button[type=submit] {
      background: linear-gradient(135deg, #00e676, #00c853);
    }

    button[type=button] {
      background: #e0e0e0;
      color: #333;
    }

    #msg {
      margin-top: 20px;
      font-size: 18px;
      color: #1565c0;
      font-weight: 600;
    }
  </style>
</head>
<body>
  <div class='container'>
    <h2>📶 Wi-Fi Settings</h2>
    <form id="wifiForm">
      <input name="ssid" id="ssid" type="text" placeholder="Tên Wi-Fi (SSID)" required><br>
      
      <input name="password" id="pass" type="password" placeholder="Mật khẩu" required><br>
      <button type="submit">Connect</button>
      <button type="button" onclick="window.location='/'">Back</button>
    </form>
    <div id="msg"></div>
  </div>

  <script>
    document.getElementById('wifiForm').onsubmit = function(e) {
      e.preventDefault();
      let ssid = document.getElementById('ssid').value;
      let pass = document.getElementById('pass').value;
      let msgDiv = document.getElementById('msg');
      
      msgDiv.innerText = 'Connecting...'; // Thông báo ngay lập tức

      fetch('/connect?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass))
        .then(r => r.text())
        .then(msg => {
          msgDiv.innerText = msg; // Hiển thị thông báo từ server
        })
        .catch(err => {
           msgDiv.innerText = 'Kết nối thành công!!.';
console.error(err);
        });
    };
  </script>
</body>
</html>
  )rawliteral";
}

// ========== Handlers ==========
void handleRoot() { server.send(200, "text/html", mainPage()); }

void handleToggle()
{
 setupPinMode();
 PWMdetach();
  int led = server.arg("led").toInt();
  if (led == 1)
  {
    led1_state = !led1_state;
    digitalWrite(LED1_PIN,led1_state);
    Serial.println("YOUR CODE TO CONTROL LED1");
  }
  else if (led == 2)
  {
    led2_state = !led2_state;
    digitalWrite(LED2_PIN,led2_state);
    Serial.println("YOUR CODE TO CONTROL LED2");
   // Serial.println(digitalRead(LED2_PIN));
  }
  server.send(200, "application/json",
              "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}");

}

void handleSensors()
{

  MLResult current_data;
  float temp = 0;
  float humidity = 0;
  float anom_score = 0;
  String anomaly_type = "WAITING";

  if(xQueuePeek(xQueueLatestState, &current_data, 0) == pdTRUE)
  {
      temp = current_data.temperature;
      humidity = current_data.humidity;
      anom_score = current_data.inference_result;
      anomaly_type = current_data.anomaly_type;
  }
 String json = "{\"temp\":" + String(temp) + 
                ",\"hum\":" + String(humidity) + 
                ",\"anom_score\":" + String(anom_score, 2) + 
                ",\"anom_type\":\"" + anomaly_type + "\"}";
  server.send(200, "application/json", json);
}

void handleSettings() { server.send(200, "text/html", settingsPage()); }

void handleConnect()
{
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}
void handleMode()
{
  int mode = server.arg("set").toInt();
  current_mode = mode;
  
  String json = "{\"mode\":" + String(mode) + "}";
  server.send(200, "application/json", json); 
}
// ========== WiFi ==========
void setupServer()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/connect", HTTP_GET, handleConnect);
  server.on("/mode",HTTP_GET,handleMode);
  server.begin();
}

void startAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  if (wifi_password.isEmpty())
  {
    WiFi.begin(wifi_ssid.c_str());
  }
  else
  {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
  Serial.println(WiFi.status());
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
}
void runMode()
{
  MLResult data;
  float temperature = 0;
  float humidity = 0;

  if (xQueuePeek(xQueueLatestState, &data, 0) == pdTRUE) {
      temperature = data.temperature;
      humidity = data.humidity;
  }
  setupPinMode();
 // PWMSetUP();
 //Serial.println("IN MODE :");
 //Serial.printf("%d\n",current_mode);
  switch(current_mode)
  {
     case 2:
     // led 1 blink 0.5hz led 2 blink 1hz 
     PWMdetach();
    
      if (millis()- lastBlink1 >= 1000) 
    {
      // LED1 0.5Hz
      //Serial.println("LED1 BLINK");
        led1_state = !led1_state;
        digitalWrite(LED1_PIN, led1_state);
        lastBlink1 = millis();
    }
    if (millis()- lastBlink2 >= 500) 
    {
      // LED2 1Hz
     //ss Serial.println("LED2 BLINK");
        led2_state = !led2_state;
        digitalWrite(LED2_PIN, led2_state);
        lastBlink2 = millis();
    
    }
       break;
      case 3:
        PWMSetUP();
     // led 1 for temperature, led 2 for humidity
        
        if(temperature < 30)
           ledcWrite(0, 64);  // duty cycle = 25
        else if(temperature <= 35)
           ledcWrite(0, 191); // duty cycle = 75%
        else ledcWrite(0,255); // duty cycle = 100%

         if (humidity < 0)    // 25 
           ledcWrite(1,64);
        else if (humidity >= 0 && humidity < 60)  // 50 
            ledcWrite(1,128);
        else if (humidity >= 60 && humidity < 80) // 75
            ledcWrite(1, 191);
        else
           ledcWrite(1,255);  //100
           
      break;
     
     default:
      break;
  }
 
}

// ========== Main task ==========
void main_server_task(void *pvParameters)
{
  pinMode(BOOT_PIN, INPUT_PULLUP);
  setupPinMode();
  startAP();
  setupServer();
  MLResult dashboard_state = {0, 0, 0, false, "WAITING"};
  SensorData sensor_in;
  MLResult ml_in;
  while (1)
  {
     server.handleClient();
    //startAP();
    if(xQueueReceive(xQueueTempHumiForMain, &sensor_in, 0) == pdTRUE)
    {
      dashboard_state.temperature = sensor_in.temperature;
      dashboard_state.humidity = sensor_in.humidity;
      xQueueOverwrite(xQueueLatestState, &dashboard_state);
    }
    if(xQueueReceive(xQueueForMainServer, &ml_in, 0) == pdTRUE)
    {
      dashboard_state.inference_result = ml_in.inference_result;
      dashboard_state.anomaly_detected = ml_in.anomaly_detected;
      dashboard_state.anomaly_type = ml_in.anomaly_type;
      xQueueOverwrite(xQueueLatestState, &dashboard_state);
    }
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW)
      {
        if (!isAPMode)
        {
          startAP();
        setupServer();
        }
      }
    }

    // STA Mode
    if (connecting)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; // Internet access

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
         connecting = false;
      }
      else if (millis() - connect_start_ms > 10000)
      { // timeout 10s
          Serial.println("WiFi connect failed! Back to AP.");
          startAP();
          setupServer();
          connecting = false;
          isWifiConnected = false;
      }
    }
  runMode();

    vTaskDelay(20); // avoid watchdog reset
  }
}