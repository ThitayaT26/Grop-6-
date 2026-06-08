const int RELAY_PIN = 25;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);

  // ปิดรีเลย์เริ่มต้น
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {

  // เปิดรีเลย์
  Serial.println("Relay ON");
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000);

  // ปิดรีเลย์
  Serial.println("Relay OFF");
  digitalWrite(RELAY_PIN, LOW);
  delay(5000);
}
