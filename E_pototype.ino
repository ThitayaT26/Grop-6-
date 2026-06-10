#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"

// ========== CONFIGURATION ==========
const char* SSID = "MoBoi";     
const char* PASSWORD = "99996666"; 
const char* FIREBASE_URL = "https://smart-haven-88971-default-rtdb.firebaseio.com/logs.json"; 
const char* LINE_CHANNEL_ACCESS_TOKEN = "NG11nfkHX6ATNEBHOSr6NYjKDhqA+sI+3+lcbtq6GjLX9ZJU9D7fKLUC+0a5wnkAcU663xPOwBPKS2VLIb7NxDrqu4O4vcAshmM1p9jMD6aCEnGJ+KvWY6BX6n/Ub6lYXXQZXNvfXeMGxmIwvkCSiwdB04t89/1O/w1cDnyilFU="; 
const char* USER_ID = "Uf1968d3ba22643a130c672a8169d15aa"; 

// ========== HARDWARE PINS ==========
const int RELAY_PIN = 25;    // รีเลย์
const int MQ2_PIN = 34;      // เซนเซอร์ควัน
const int FLAME_PIN = 4;     // เซนเซอร์เปลวไฟ

// ========== THRESHOLDS ==========
const int SMOKE_THRESHOLD = 2000;

// ========== TIMING ==========
unsigned long previousMillisWiFi = 0;
const unsigned long wifiCheckInterval = 10000;
unsigned long previousSensorMillis = 0;
const unsigned long sensorReadInterval = 1000;

// ========== NTP ==========
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200;  // UTC+7
const int daylightOffset_sec = 0;

// ========== VARIABLE ==========
bool lastAlertSent = false;

// ========== ฟังก์ชันเพิ่ม Timestamp ใน Serial Print ==========
void printWithTimestamp(const char* message, const char* level = "INFO") {
  struct tm timeinfo;
  char timeStr[20];
  
  if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  } else {
    sprintf(timeStr, "00:00:00");
  }
  
  Serial.printf("[%s] [%s] %s\n", timeStr, level, message);
}

// ========== FUNCTION: GET TIME ==========
String getActualCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    printWithTimestamp("NTP sync failed - using default time", "WARN");
    return "1970-01-01 00:00:00";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// ========== FUNCTION: SETUP WIFI ==========
void setupWiFi() {
  Serial.println("\n-------------------------------------------");
  printWithTimestamp("Connecting to Wi-Fi...", "INFO");
  WiFi.begin(SSID, PASSWORD);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    printWithTimestamp("Wi-Fi connected successfully!", "INFO");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // รอ NTP sync
    int retry = 0;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo) && retry < 10) {
      delay(500);
      retry++;
    }
    
    if (retry < 10) {
      printWithTimestamp("NTP time synced!", "INFO");
    } else {
      printWithTimestamp("NTP sync timeout!", "WARN");
    }
  } else {
    printWithTimestamp("Wi-Fi connection failed - check SSID/PW", "ERROR");
  }
  Serial.println("-------------------------------------------");
}

// ========== FUNCTION: KEEP WIFI ALIVE ==========
void keepWiFiAlive() {
  unsigned long currentMillis = millis();
  if (WiFi.status() != WL_CONNECTED && (currentMillis - previousMillisWiFi >= wifiCheckInterval)) {
    printWithTimestamp("Wi-Fi lost, reconnecting...", "WARN");
    WiFi.disconnect();
    WiFi.begin(SSID, PASSWORD);
    previousMillisWiFi = currentMillis;
  }
}

// ========== FUNCTION: PUSH TO FIREBASE ==========
void pushLogToFirebase(int smokeValue, bool fireStatus, String timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    printWithTimestamp("Firebase: No WiFi connection", "ERROR");
    return;
  }
  
  HTTPClient http;
  http.begin(FIREBASE_URL);
  http.addHeader("Content-Type", "application/json");
  
  String jsonPayload = "{\"smoke_level\":" + String(smokeValue) +
                       ", \"fire_incident\":" + String(fireStatus ? "true" : "false") +
                       ", \"timestamp\":\"" + timestamp + "\"}";
  
  printWithTimestamp(("Firebase Payload: " + jsonPayload).c_str(), "DEBUG");
  
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    printWithTimestamp(("Firebase success - Code: " + String(httpResponseCode)).c_str(), "INFO");
  } else {
    printWithTimestamp(("Firebase failed - Code: " + String(httpResponseCode)).c_str(), "ERROR");
  }
  
  http.end();  // สำคัญ: ปิด connection
}

// ========== FUNCTION: SEND LINE NOTIFICATION ==========
void sendLineNotification(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    printWithTimestamp("LINE: No WiFi connection", "ERROR");
    return;
  }
  
  HTTPClient https;
  String url = "https://api.line.me/v2/bot/message/push";
  https.begin(url);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", "Bearer " + String(LINE_CHANNEL_ACCESS_TOKEN));
  
  String payload = "{\"to\":\"" + String(USER_ID) + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";
  
  printWithTimestamp(("LINE Payload length: " + String(payload.length())).c_str(), "DEBUG");
  
  int httpCode = https.POST(payload);
  
  if (httpCode > 0) {
    printWithTimestamp(("LINE notification sent - Code: " + String(httpCode)).c_str(), "INFO");
  } else {
    printWithTimestamp(("LINE failed - Code: " + String(httpCode)).c_str(), "ERROR");
  }
  
  https.end();
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printWithTimestamp("System starting up...", "INFO");
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  
  setupWiFi();
}

// ========== LOOP ==========
void loop() {
  keepWiFiAlive();
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousSensorMillis >= sensorReadInterval) {
    previousSensorMillis = currentMillis;
    
    // อ่านค่าเซนเซอร์
    int smokeValue = analogRead(MQ2_PIN);
    bool fireDetected = (digitalRead(FLAME_PIN) == HIGH);
    
    String timestamp = getActualCurrentTime();
    
    printWithTimestamp(("Smoke: " + String(smokeValue) + ", Fire: " + String(fireDetected ? "YES" : "NO")).c_str(), "INFO");
    
    // Push ไป Firebase
    pushLogToFirebase(smokeValue, fireDetected, timestamp);
    
    // ตรวจจับเหตุการณ์
    if (smokeValue > SMOKE_THRESHOLD || fireDetected) {
      if (!lastAlertSent) {
        String alertMsg = "⚠️ ALERT!\n";
        if (smokeValue > SMOKE_THRESHOLD) alertMsg += "🔥 High Smoke: " + String(smokeValue) + "\n";
        if (fireDetected) alertMsg += "🔥 Fire Detected!\n";
        alertMsg += "Time: " + timestamp;
        
        sendLineNotification(alertMsg);
        digitalWrite(RELAY_PIN, HIGH);
        lastAlertSent = true;
        
        printWithTimestamp("ALERT TRIGGERED - Relay ON", "WARN");
      }
    } else {
      if (lastAlertSent) {
        digitalWrite(RELAY_PIN, LOW);
        lastAlertSent = false;
        printWithTimestamp("Alert cleared - Relay OFF", "INFO");
      }
    }
  }
}
