// buzzer.ino
// พาร์ทที่ 4: ระบบแจ้งเตือนและระบบดับเพลิงอัตโนมัติ
#define BUZZER_PIN         3   // ขา PWM สำหรับ Buzzer
#define LED_RED_PIN        4   // LED แจ้งเตือนสถานะอันตราย
#define LED_YELLOW_PIN     5   // LED แจ้งเตือนสถานะเตือนภัย
#define RELAY_FIRE_PIN     6   // รีเลย์ควบคุมระบบดับเพลิง (水泵/สปริงเกอร์)

// รวมตัวแปรสถานะจากเซ็นเซอร์อื่น ๆ
extern int gasLevel;      // จาก mq2_sensor
extern bool smokeDetected; // จาก smoke_sensor  
extern bool flameDetected; // จาก flame_sensor

bool fireSuppressionActive = false;

void setup_alert() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(RELAY_FIRE_PIN, OUTPUT);
  
  digitalWrite(RELAY_FIRE_PIN, LOW);  // ปิดระบบดับเพลิงตอนเริ่มต้น
  
  Serial.println("[Alert System] Ready");
  Serial.println("[Fire Suppression] Disabled");
}

void loop_alert() {
  // ตรวจจับสถานการณ์อันตราย
  bool isCritical = (gasLevel == 2) || flameDetected || (smokeDetected && gasLevel >= 1);
  bool isWarning = (gasLevel == 1) || smokeDetected;
  
  if (isCritical) {
    // สถานะอันตราย: เสียงดัง+LEDแดงติด+เปิดระบบดับเพลิง
    tone(BUZZER_PIN, 2500);           // เสียงดัง 2500Hz
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_YELLOW_PIN, LOW);
    
    if (!fireSuppressionActive) {
      fireSuppressionActive = true;
      digitalWrite(RELAY_FIRE_PIN, HIGH);
      Serial.println("[FIRE SUPPRESSION] ACTIVATED! Pump/Sprinkler ON");
    }
  }
  else if (isWarning) {
    // สถานะเตือนภัย: เสียงกระพริบ+LEDเหลืองกระพริบ
    static unsigned long lastBeep = 0;
    if (millis() - lastBeep > 500) {
      lastBeep = millis();
      digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
      digitalWrite(LED_YELLOW_PIN, !digitalRead(LED_YELLOW_PIN));
    }
    digitalWrite(LED_RED_PIN, LOW);
    
    // ถ้าดับเพลิงกำลังทำงาน แต่สถานะลดลงเป็นแค่ warning ให้ปิดระบบดับเพลิง
    if (fireSuppressionActive) {
      fireSuppressionActive = false;
      digitalWrite(RELAY_FIRE_PIN, LOW);
      Serial.println("[Fire Suppression] Deactivated");
    }
  }
  else {
    // สถานะปกติ: ไม่มีเสียง, LED ดับ
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    
    if (fireSuppressionActive) {
      fireSuppressionActive = false;
      digitalWrite(RELAY_FIRE_PIN, LOW);
    }
  }
  
  delay(100);
}

// ฟังก์ชันทดสอบระบบดับเพลิง
void testFireSuppression() {
  Serial.println("Testing fire suppression system...");
  digitalWrite(RELAY_FIRE_PIN, HIGH);
  delay(2000);
  digitalWrite(RELAY_FIRE_PIN, LOW);
  Serial.println("Test complete");
}
