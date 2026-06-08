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
const int RELAY_PIN = 25;    // รีเลย์ (ใช้ Open Drain Mode)
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

// ========== VARIABLE FOR LINE RETRY ==========
bool lastAlertSent = false;

// ========== FUNCTION: GET TIME ==========
String getActualCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "UNKNOWN_TIME";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// ========== FUNCTION: SETUP WIFI ==========
void setupWiFi() {
  Serial.println("\n-------------------------------------------");
  Serial.print("📶 Connecting to Wi-Fi... ");
  WiFi.begin(SSID, PASSWORD);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi connected!");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("⏰ Time synced!");
  } else {
    Serial.println("\n❌ Wi-Fi failed - ตรวจสอบ SSID และ Password");
  }
  Serial.println("-------------------------------------------");
}

// ========== FUNCTION: KEEP WIFI ALIVE ==========
void keepWiFiAlive() {
  unsigned long currentMillis = millis();
  if (WiFi.status() != WL_CONNECTED && (currentMillis - previousMillisWiFi >= wifiCheckInterval)) {
    Serial.println("\n📡 Reconnecting Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(SSID, PASSWORD);
    previousMillisWiFi = currentMillis;
  }
}

// ========== FUNCTION: PUSH TO FIREBASE ==========
void pushLogToFirebase(int smokeValue, bool fireStatus, String timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("   ❌ Firebase: ไม่มี WiFi");
    return;
  }
  
  HTTPClient http;
  http.begin(FIREBASE_URL);
  http.addHeader("Content-Type", "application/json");
  
  String jsonPayload = "{\"smoke_level\":" + String(smokeValue) +
                       ", \"fire_incident\":" + String(fireStatus ? "true" : "false") +
                       ", \"timestamp\":\"" + timestamp + "\"}";
  
  Serial.print("   📤 Firebase Payload: ");
  Serial.println(jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);
