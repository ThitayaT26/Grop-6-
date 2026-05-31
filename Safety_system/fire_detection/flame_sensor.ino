// flame_sensor.ino
// พาร์ทที่ 2: ตรวจจับเปลวไฟด้วยเซ็นเซอร์ตรวจจับแสงอินฟราเรด (Flame Sensor)
#define FLAME_SENSOR_PIN A1 // ขาอ่านค่า analog
#define FLAME_THRESHOLD 200 // ค่า threshold

int flameValue = 0;
bool flameDetected = false;
void setup_flame(){
  pinMode(FLAME_SENSOR_PIN,INPUT);
  Serial.println("[Flame Sensor]Ready");
}

void loop_flame(){
  flameValue = analogRead("[FLAME_SENSOR_PIN);
  //เช็นเอร์ตรวจจับเปลวไฟ ค่ายิ่งน้อยไฟยิ่งแรง
  if (flameValue<FLAME_THRESHOLD){
    if (!flameDetected){
      flameDetected = true;
      Serial.print("[FLAME ALERT]Fire detected! Value = ");
      Serial.println(flameValue);
      }
  }
  else{
    if (flameDetected){
      flameDetected = false;
      Serial.println("[Flame]No fire");
    }
  }
  delay(200);
}

int getFlameValue(){
  return analogRead(FLAME_SENSOR_PIN);
}

bool isFlameDetected(){
  return (analogRead(FLAME_SENSOR_PIN) < FLAME_THRESHOLD);
}
