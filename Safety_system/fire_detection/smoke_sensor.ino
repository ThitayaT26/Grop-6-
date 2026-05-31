// smoke_sensor.ino
// พาร์ทที่ 1: ตรวจจับควันจากเพลิงไหม้ ด้วยเซ็นเซอร์วัดควัน (เช่น MQ-2 หรือ MQ-135)

#define SMOKE_SENSOR_PIN   A0   // ขาอ่านค่า analog จากเซ็นเซอร์ควัน
#define SMOKE_THRESHOLD    300  // ค่า threshold ที่เริ่มแจ้งเตือน (ปรับตามเซ็นเซอร์)

int smokeValue = 0;
bool smokeDetected = false;

void setup_smoke() {
  pinMode(SMOKE_SENSOR_PIN, INPUT);
  Serial.println("[Smoke Sensor] Ready");
}

void loop_smoke() {
  smokeValue = analogRead(SMOKE_SENSOR_PIN);
  
  if (smokeValue > SMOKE_THRESHOLD) {
    if (!smokeDetected) {
      smokeDetected = true;
      Serial.print("[SMOKE ALERT] Detected! Value = ");
      Serial.println(smokeValue);
    }
  } else {
    if (smokeDetected) {
      smokeDetected = false;
      Serial.println("[Smoke] Normal");
    }
  }
  
  delay(500);  // อ่านค่าทุก 0.5 วินาที
}

int getSmokeValue() {
  return analogRead(SMOKE_SENSOR_PIN);
}

bool isSmokeDetected() {
  return (analogRead(SMOKE_SENSOR_PIN) > SMOKE_THRESHOLD);
}
