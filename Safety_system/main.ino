// main.ino
// พาร์ทที่ 5: ไฟล์หลักสำหรับรวมทุกเซ็นเซอร์และระบบแจ้งเตือนเข้าด้วยกัน

// include ไฟล์ย่อยทั้งหมด (ใน Arduino IDE ให้สร้างแท็บแยก)
#include "smoke_sensor.ino"
#include "flame_sensor.ino"  
#include "mq2_sensor.ino"
#include "buzzer.ino"

// ตัวแปร global ที่ใช้ร่วมกัน
int gasLevel = 0;
bool smokeDetected = false;
bool flameDetected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("======================================");
  Serial.println("Gas & Fire Detection System with Auto Fire Suppression");
  Serial.println("======================================");
  
  // เรียก setup ของแต่ละพาร์ท
  setup_smoke();
  setup_flame();
  setup_mq2();
  setup_alert();
  
  Serial.println("[SYSTEM] All sensors ready. Monitoring started...");
}

void loop() {
  // อ่านค่าจากเซ็นเซอร์ทุกตัว
  loop_smoke();     // อัปเดต smokeDetected
  loop_flame();     // อัปเดต flameDetected
  loop_mq2();       // อัปเดต gasLevel
  loop_alert();     // ระบบแจ้งเตือนและดับเพลิงอัตโนมัติ
  
  // แสดงสถานะรวมทุก 2 วินาที
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 2000) {
    lastStatus = millis();
    Serial.print("Status | Smoke:");
    Serial.print(smokeDetected ? "YES" : "NO");
    Serial.print(" | Flame:");
    Serial.print(flameDetected ? "YES" : "NO");
    Serial.print(" | Gas Level:");
    Serial.print(gasLevel);
    Serial.println();
  }
}
