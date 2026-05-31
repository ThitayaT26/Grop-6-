/**
 ******************************************************************************
 * @file           : Gas_Leak_Fire_Detection_System.ino
 * @brief          : ระบบตรวจจับแก๊สรั่ว + ตรวจจับควัน/เปลวไฟ + ระบบดับเพลิงอัตโนมัติ
 * @board          : Arduino Uno / Nano / Mega / ESP32
 ******************************************************************************
 */

// ======================== พาร์ทที่ 1: ประกาศขาเชื่อมต่อ Hardware ========================
// ขาเซ็นเซอร์
#define SMOKE_SENSOR_PIN     A0    // เซ็นเซอร์ตรวจจับควัน (MQ-2 หรือ MQ-135)
#define FLAME_SENSOR_PIN     A1    // เซ็นเซอร์ตรวจจับเปลวไฟ (KY-026)
#define MQ2_SENSOR_PIN       A2    // เซ็นเซอร์ตรวจจับแก๊สรั่ว (MQ-2)

// ขาเอาต์พุตสำหรับแจ้งเตือนและควบคุม
#define BUZZER_PIN           3     // Buzzer (PWM)
#define LED_RED_PIN          4     // LED สีแดง (สถานะอันตราย)
#define LED_YELLOW_PIN       5     // LED สีเหลือง (สถานะเตือนภัย)
#define LED_GREEN_PIN        6     // LED สีเขียว (สถานะปกติ)
#define RELAY_FIRE_PIN       7     // รีเลย์ควบคุมระบบดับเพลิง (水泵/สปริงเกอร์/พัดลม)

// ======================== พาร์ทที่ 2: ค่า Threshold และตัวแปร ========================
// ค่า Threshold สำหรับเซ็นเซอร์แต่ละตัว
#define SMOKE_THRESHOLD      300   // ค่า ADC ที่เริ่มแจ้งเตือนควัน
#define FLAME_THRESHOLD      200   // ค่า ADC ที่ตรวจจับเปลวไฟ (ยิ่งต่ำยิ่งมีไฟ)
#define MQ2_WARN_THRESHOLD   200   // ค่าเตือนภัยแก๊สรั่ว
#define MQ2_ALARM_THRESHOLD  400   // ค่าอันตรายแก๊สรั่ว

// เวลาต่างๆ
#define SENSOR_READ_INTERVAL 500   // อ่านค่าเซ็นเซอร์ทุก 500ms
#define BUZZER_BEAT_INTERVAL 300   // เสียงเตือนกระพริบในโหมด Warning (300ms)
#define SUPPRESSION_DELAY_MS 5000  // ระบบดับเพลิงทำงานค้าง 5 วินาทีหลังจากสถานการณ์กลับสู่ปกติ

// ตัวแปรเก็บค่าสถานะ
int smokeValue = 0;
int flameValue = 0;
int mq2Value = 0;

// ตัวแปรสถานะปกติ
int gasLevel = 0;           // 0=normal, 1=warning, 2=alarm
bool smokeDetected = false;
bool flameDetected = false;

// ตัวแปรสำหรับ Non-blocking timing
unsigned long lastSensorRead = 0;
unsigned long lastBuzzToggle = 0;
unsigned long suppressionEndTime = 0;
bool isBuzzerOn = false;
bool fireSuppressionActive = false;

// ======================== พาร์ทที่ 3: ฟังก์ชันตั้งค่าเริ่มต้น (setup) ========================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println(F("=========================================="));
  Serial.println(F("Gas & Fire Detection System"));
  Serial.println(F("with Auto Fire Suppression"));
  Serial.println(F("=========================================="));
  
  // ตั้งค่าขาเซ็นเซอร์
  pinMode(SMOKE_SENSOR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(MQ2_SENSOR_PIN, INPUT);
  
  // ตั้งค่าขาเอาต์พุต
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(RELAY_FIRE_PIN, OUTPUT);
  
  // ตั้งค่าเริ่มต้นให้อุปกรณ์ทั้งหมดอยู่ในสถานะ OFF
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, HIGH);   // LED เขียวติดแสดงว่าระบบพร้อมทำงาน
  digitalWrite(RELAY_FIRE_PIN, LOW);   // ปิดระบบดับเพลิง
  
  Serial.println(F("[SYSTEM] All sensors initialized"));
  Serial.println(F("[SYSTEM] Warming up MQ-2 gas sensor (30 seconds)..."));
  
  // MQ-2 ต้องอุ่นเครื่อง 30 วินาทีก่อนใช้งานจริง
  for (int i = 30; i > 0; i--) {
    Serial.print(F("Warming up: "));
    Serial.print(i);
    Serial.println(F(" sec remaining"));
    delay(1000);
  }
  
  Serial.println(F("[SYSTEM] Ready! Monitoring started..."));
  Serial.println(F(""));
}

// ======================== พาร์ทที่ 4: ฟังก์ชันอ่านค่าเซ็นเซอร์ทั้งหมด ========================
void readAllSensors() {
  // อ่านค่าจาก ADC
  smokeValue = analogRead(SMOKE_SENSOR_PIN);
  flameValue = analogRead(FLAME_SENSOR_PIN);
  mq2Value = analogRead(MQ2_SENSOR_PIN);
  
  // ===== ประมวลผลควัน (Smoke) =====
  if (smokeValue > SMOKE_THRESHOLD) {
    if (!smokeDetected) {
      smokeDetected = true;
      Serial.print(F("[SMOKE] ALERT! Value: "));
      Serial.println(smokeValue);
    }
  } else {
    if (smokeDetected) {
      smokeDetected = false;
      Serial.println(F("[SMOKE] Back to normal"));
    }
  }
  
  // ===== ประมวลผลเปลวไฟ (Flame) =====
  // เซ็นเซอร์เปลวไฟ: ยิ่งมีไฟ ค่ายิ่งต่ำ (0 = มีไฟแรงมาก)
  if (flameValue < FLAME_THRESHOLD) {
    if (!flameDetected) {
      flameDetected = true;
      Serial.print(F("[FLAME] ALERT! Fire detected! Value: "));
      Serial.println(flameValue);
    }
  } else {
    if (flameDetected) {
      flameDetected = false;
      Serial.println(F("[FLAME] No fire detected"));
    }
  }
  
  // ===== ประมวลผลแก๊สรั่ว (Gas) =====
  if (mq2Value >= MQ2_ALARM_THRESHOLD) {
    if (gasLevel != 2) {
      gasLevel = 2;
      Serial.print(F("[GAS] CRITICAL ALARM! Value: "));
      Serial.println(mq2Value);
    }
  }
  else if (mq2Value >= MQ2_WARN_THRESHOLD) {
    if (gasLevel != 1) {
      gasLevel = 1;
      Serial.print(F("[GAS] WARNING! Gas detected. Value: "));
      Serial.println(mq2Value);
    }
  }
  else {
    if (gasLevel != 0) {
      gasLevel = 0;
      Serial.println(F("[GAS] Normal"));
    }
  }
}

// ======================== พาร์ทที่ 5: ฟังก์ชันประเมินสถานการณ์โดยรวม ========================
// คืนค่า: 0 = ปกติ, 1 = เตือนภัย, 2 = อันตราย
int getOverallStatus() {
  // อันตรายระดับ 2: มีแก๊สรั่วระดับอันตราย OR ตรวจเจอเปลวไฟ
  if (gasLevel == 2 || flameDetected) {
    return 2;
  }
  // เตือนภัยระดับ 1: มีแก๊สรั่วระดับเตือนภัย OR มีควัน
  else if (gasLevel == 1 || smokeDetected) {
    return 1;
  }
  return 0;
}

// ======================== พาร์ทที่ 6: ระบบแจ้งเตือน (LED + Buzzer) ========================
void updateAlertSystem(int status, unsigned long currentTime) {
  switch (status) {
    case 2:  // สถานะอันตราย (CRITICAL)
      // LED สีแดงติดค้าง
      digitalWrite(LED_RED_PIN, HIGH);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, LOW);
      
      // เสียง Buzzer ดังต่อเนื่อง (ไม่กระพริบ)
      digitalWrite(BUZZER_PIN, HIGH);
      isBuzzerOn = true;
      break;
      
    case 1:  // สถานะเตือนภัย (WARNING)
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, LOW);
      
      // LED สีเหลืองและ Buzzer กระพริบเป็นจังหวะ
      if ((currentTime - lastBuzzToggle) >= BUZZER_BEAT_INTERVAL) {
        lastBuzzToggle = currentTime;
        isBuzzerOn = !isBuzzerOn;
        digitalWrite(LED_YELLOW_PIN, isBuzzerOn);
        digitalWrite(BUZZER_PIN, isBuzzerOn ? HIGH : LOW);
      }
      break;
      
    default:  // สถานะปกติ (NORMAL)
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
      isBuzzerOn = false;
      lastBuzzToggle = currentTime;
      break;
  }
}

// ======================== พาร์ทที่ 7: ระบบดับเพลิงอัตโนมัติ (Auto Fire Suppression) ========================
void updateFireSuppression(int status, unsigned long currentTime) {
  static int lastStatus = 0;
  
  if (status == 2) {
    // สถานะอันตราย: เปิดระบบดับเพลิงทันที
    if (!fireSuppressionActive) {
      fireSuppressionActive = true;
      digitalWrite(RELAY_FIRE_PIN, HIGH);
      Serial.println(F("[FIRE SUPPRESSION] >>> ACTIVATED! Pump/Sprinkler ON <<<"));
    }
    // รีเซ็ตเวลาปิดระบบทุกครั้งที่ยังมีอันตราย
    suppressionEndTime = currentTime + SUPPRESSION_DELAY_MS;
  }
  else if (status == 1 && lastStatus == 2) {
    // สถานะเปลี่ยนจากอันตราย -> เตือนภัย: ยังไม่ปิดทันที รอให้สถานการณ์นิ่งก่อน
    // ไม่ต้องทำอะไร รอ timeout
  }
  else if (status == 0 && fireSuppressionActive) {
    // สถานะปกติ: ถ้าระบบดับเพลิงกำลังทำงาน ให้รอเวลาครบก่อนปิด
    if (currentTime >= suppressionEndTime) {
      fireSuppressionActive = false;
      digitalWrite(RELAY_FIRE_PIN, LOW);
      Serial.println(F("[FIRE SUPPRESSION] >>> Deactivated <<<"));
    }
  }
  
  lastStatus = status;
}

// ======================== พาร์ทที่ 8: ฟังก์ชันแสดงสถานะทาง Serial Monitor ========================
void printStatusReport(int status) {
  static unsigned long lastReport = 0;
  unsigned long currentTime = millis();
  
  // แสดงรายงานทุก 3 วินาที
  if ((currentTime - lastReport) >= 3000) {
    lastReport = currentTime;
    
    Serial.print(F("========================================\n"));
    Serial.print(F("STATUS: "));
    switch (status) {
      case 2: Serial.print(F("[!! CRITICAL !!!]")); break;
      case 1: Serial.print(F("[!! WARNING !!!]")); break;
      default: Serial.print(F("[ OK ]")); break;
    }
    
    Serial.print(F(" | Smoke:"));
    Serial.print(smokeDetected ? F("YES") : F("NO"));
    Serial.print(F(" (")); Serial.print(smokeValue); Serial.print(F(")"));
    
    Serial.print(F(" | Flame:"));
    Serial.print(flameDetected ? F("YES") : F("NO"));
    Serial.print(F(" (")); Serial.print(flameValue); Serial.print(F(")"));
    
    Serial.print(F(" | Gas:"));
    if (gasLevel == 2) Serial.print(F("ALARM"));
    else if (gasLevel == 1) Serial.print(F("WARN"));
    else Serial.print(F("OK"));
    Serial.print(F(" (")); Serial.print(mq2Value); Serial.print(F(")"));
    
    Serial.print(F(" | Suppression:"));
    Serial.print(fireSuppressionActive ? F("ON") : F("OFF"));
    
    Serial.println(F("\n========================================"));
  }
}

// ======================== พาร์ทที่ 9: ฟังก์ชันทดสอบระบบ (Test Mode) ========================
void runSystemTest() {
  Serial.println(F("\n========== SYSTEM TEST START =========="));
  
  // ทดสอบ LED ทั้งหมด
  Serial.println(F("Testing LEDs..."));
  digitalWrite(LED_GREEN_PIN, HIGH); delay(500);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, HIGH); delay(500);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH); delay(500);
  digitalWrite(LED_RED_PIN, LOW);
  
  // ทดสอบ Buzzer
  Serial.println(F("Testing Buzzer..."));
  tone(BUZZER_PIN, 2000); delay(500);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, 1000); delay(500);
  noTone(BUZZER_PIN);
  
  // ทดสอบรีเลย์ดับเพลิง
  Serial.println(F("Testing Fire Suppression Relay..."));
  digitalWrite(RELAY_FIRE_PIN, HIGH); delay(1000);
  digitalWrite(RELAY_FIRE_PIN, LOW);
  
  Serial.println(F("========== SYSTEM TEST COMPLETE ==========\n"));
  delay(1000);
  
  // กลับสู่สถานะปกติ
  digitalWrite(LED_GREEN_PIN, HIGH);
}

// ======================== พาร์ทที่ 10: ฟังก์ชันหลัก loop() ========================
void loop() {
  unsigned long currentTime = millis();
  
  // อ่านค่าเซ็นเซอร์ทุก SENSOR_READ_INTERVAL ms
  if ((currentTime - lastSensorRead) >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentTime;
    readAllSensors();
  }
  
  // ประเมินสถานการณ์โดยรวม
  int overallStatus = getOverallStatus();
  
  // อัปเดตระบบแจ้งเตือน (LED + Buzzer)
  updateAlertSystem(overallStatus, currentTime);
  
  // อัปเดตระบบดับเพลิงอัตโนมัติ
  updateFireSuppression(overallStatus, currentTime);
  
  // แสดงสถานะทาง Serial Monitor
  printStatusReport(overallStatus);
  
  // ตรวจสอบคำสั่งจาก Serial (สำหรับ debug หรือทดสอบระบบ)
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 't' || cmd == 'T') {
      runSystemTest();
    }
    else if (cmd == 'r' || cmd == 'R') {
      Serial.println(F("Manual reset: Turning off fire suppression"));
      fireSuppressionActive = false;
      digitalWrite(RELAY_FIRE_PIN, LOW);
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    }
    else if (cmd == 's' || cmd == 'S') {
      Serial.println(F("Manual suppression test: Turning relay ON for 2 sec"));
      digitalWrite(RELAY_FIRE_PIN, HIGH);
      delay(2000);
      digitalWrite(RELAY_FIRE_PIN, LOW);
    }
  }
  
  //  delay เล็กน้อยเพื่อป้องกัน CPU ทำงานหนักเกินไป
  delay(50);
}
