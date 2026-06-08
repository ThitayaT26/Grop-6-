const int FLAME_PIN = 4;

void setup() {
    Serial.begin(115200);
    pinMode(FLAME_PIN, INPUT);
}

void loop() {

    bool fireStatus = (digitalRead(FLAME_PIN) == LOW);

    if (fireStatus) {
        Serial.println("🔥 ตรวจพบเปลวไฟ!");
    } else {
        Serial.println("✅ ไม่พบไฟ");
    }

    delay(1000);
}
