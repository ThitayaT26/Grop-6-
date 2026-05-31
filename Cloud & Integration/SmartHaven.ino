#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"

// =========================================================================
// 1. ส่วนตั้งค่าคอนฟิกส่วนกลาง (Global Configuration)
// =========================================================================
const char* SSID = "MoBoi";     
const char* PASSWORD = "99996666"; 

const char* FIREBASE_URL = "https://smart-haven-88971-default-rtdb.firebaseio.com/logs.json"; 
const char* LINE_CHANNEL_ACCESS_TOKEN = "3v5YyVZrSySKIhZd1xiAiwlT7OpHWzYu2bpJ5Gi7q27YD7ETmPhlAdY9JHyMyUZ9cU663xPOwBPKS2VLIb7NxDrqu4O4vcAshmM1p9jMD6YYAPqqiQbCf/ecnidqjdA1xfieUXpPs3ddby/81Lwv0QdB04t89/1O/w1cDnyilFU="; 
const char* USER_ID = "Uf1968d3ba22643a130c672a8169d15aa"; 

const int RELAY_PIN = 12; 
unsigned long previousMillisWiFi = 0;
const unsigned long wifiCheckInterval = 10000; 

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200; 
const int   daylightOffset_sec = 0;

// =========================================================================
// 2. ฟังก์ชันดึงเวลาปัจจุบันจากเน็ต (ดึงมาจัดรูปแบบหล่อๆ)
// =========================================================================
String getActualCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "UNKNOWN_TIME"; 
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// =========================================================================
// 3. ส่วนของฟังก์ชันย่อยระบบ (Sub-functions)
// =========================================================================

void setupWiFi() {
  Serial.print("Connecting to Wi-Fi... ");
  WiFi.begin(SSID, PASSWORD);
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 15) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[CONNECTED] ต่อเน็ตสำเร็จแล้ว!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
   
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("[TIME] ซิงค์เวลาปัจจุบันกับ NTP Server สำเร็จ!");
  } else {
    Serial.println("\n[OFFLINE] ต่อเน็ตไม่สำเร็จ");
  }
}

void keepWiFiAlive() {
  unsigned long currentMillis = millis();
  if (WiFi.status() != WL_CONNECTED && (currentMillis - previousMillisWiFi >= wifiCheckInterval)) {
    WiFi.disconnect();
    WiFi.begin(SSID, PASSWORD);
    previousMillisWiFi = currentMillis;
  }
}

void pushLogToFirebase(float temperature, bool fireStatus, String timestamp) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(FIREBASE_URL);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"temperature\":" + String(temperature) + 
                       ", \"fire_incident\":" + String(fireStatus ? "true" : "false") + 
                       ", \"timestamp\":\"" + timestamp + "\"}";

  int httpResponseCode = http.POST(jsonPayload);
  Serial.print("   └─ Firebase Response Code: ");
  Serial.println(httpResponseCode);
  http.end(); 
}

void sendLineAlert(float targetTemp, String timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("   └─ [LINE] ข้ามการส่ง: ไวไฟไม่ได้เชื่อมต่อ");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); 
  
  HTTPClient http;
  http.setReuse(false); 
  
  Serial.println("   └─ [LINE] กำลังเชื่อมต่อเซิร์ฟเวอร์ api.line.me...");
  http.begin(client, "https://api.line.me/v2/bot/message/push");
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(LINE_CHANNEL_ACCESS_TOKEN));
  
  String msgText = "🚨 [SMART HAVEN - RED ALERT] 🚨\\n";
  msgText += "ตรวจพบอัคคีภัยและความร้อนสูงบริเวณเตาครัวครัวของตาสงวน!\\n";
  msgText += "📈 อุณหภูมิหน้าเตา: " + String(targetTemp) + " °C\\n";
  msgText += "⏰ เวลาเกิดเหตุ: " + timestamp + "\\n";
  msgText += "🫧 Status: ระบบปล่อยโฟมดับเพลิงอัตโนมัติทำงานแล้วในเวลานี้";

  String jsonMessage = "{\"to\":\"" + String(USER_ID) + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + msgText + "\"}]}";

  Serial.println("   └─ [LINE] กำลังยิง POST Request ภาษาไทยพร้อมเวลาจริง...");
  int httpResponseCode = http.POST(jsonMessage);
  
  Serial.print("   └─ LINE API Response Code: ");
  Serial.println(httpResponseCode);
  
  if (httpResponseCode != 200) {
    String response = http.getString();
    Serial.println("   └─ LINE Error Details: " + response);
  } else {
    Serial.println("   └─ [LINE] LINE Message Sent Successfully!");
  }
  http.end();
}

// =========================================================================
// 4. ฟังก์ชันหลัก (Main Control Loops)
// =========================================================================

void setup() {
  Serial.begin(115200);
  delay(1000); 
  setupWiFi(); 
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  
  Serial.println("\n=============================================");
  Serial.println("🔥 SMART HAVEN - REAL-TIME CLOCK MOCK READY 🔥");
  Serial.println("พิมพ์คำว่า 'FIRE' แล้วกด Enter");
  Serial.println("=============================================");
}

void loop() {
  keepWiFiAlive(); 
  
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');
    inputString.trim(); 
    
    if (inputString == "FIRE") {
      Serial.println("\n[!] ALERT: ตรวจพบสัญญาณจำลอง 'FIRE' จากผู้พัฒนา!");
      
      float mockTemp = 68.45; 
      String actualTime = getActualCurrentTime(); 
      
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("➔ [LOCAL ACTION] เปิดรีเลย์ปั๊มจำลองเรียบร้อย");
      
      Serial.println("➔ [CLOUD ACTION] ยิงค่าพร้อมเวลาจริง ขึ้น Firebase...");
      pushLogToFirebase(mockTemp, true, actualTime);
      
      Serial.println("➔ [API GATEWAY] ยิงข้อความภาษาไทยพร้อมเวลาจริงเข้า LINE API...");
      sendLineAlert(mockTemp, actualTime); 
      
      Serial.println("[✓] จบลูปการทำงาน หน่วงเวลาเซฟตี้ 5 วินาที...");
      delay(5000); 
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("\n[🏠] สแตนบาย... พิมพ์ 'FIRE' เพื่อทดสอบใหม่");
    }
  }
}
