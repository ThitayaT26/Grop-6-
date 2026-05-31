// mq2_sensor.ino
// พาร์ทที่ 3: ตรวจจับแก๊สรั่ว ด้วยเซ็นเซอร์ MQ-2
#defind MQ2_SENSOR_PIN A2
#defind MQ2_WARN_THRESHOLD 200
#defind MQ2_ALARM_THRESHOLD 400

int mq2Value = 0;
int gasLevel = 0;

void setup_mq2(){
  pinMode(MQ2_SENSOR_PIN,INPUT);
  Serial.println("[MQ-2 Gas Sensor]Ready");
  Serial.println("Warming up MQ-2 Sensor (30 sec)...");
  delay(30000);
  Serial.Println("[MQ-2] Ready to detect gas");
}

void loop_mq2(){
  mq2Value = analogRead(MQ2_SENSOR_PIN);
  if (mq2Value >= MQ2_ALARM_THRESHOLD){
    if (gasLevel != 2){
      gasLevel = 2;
      Serial.print("[GAS AlARM] Critical gas leak! Value =");
      Serial.println(mq2Value);
    }
  }
  else if (mq2Vale >= MQ2_WARN_THRESHOLD){
    if (gasLevel != 1){
      gasLevel = 1;
      Serial.print("[GAS WARNING] Gas detected! Value =");
      Serial.println(mq2Value);
    }
  }
  else{
    if (gasLevel != 0){
      gasLevel = 0;
      Serial.println("[Gas] Normal");
    }
  }
  delay(1000);
}

int getMq2Value(){
  return analogRead(MQ2_SENSOR_PIN);
}

int getGasLevel(){
  int val = analogRead(MQ2_SENSOR_PIN);
  if (val >= MQ2_ALARM_THRESHOLD) return 2;
  if (val >= MQ2_WARN_THRESHOLD) return 1;
  return 0;
}
