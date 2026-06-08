const int MQ2_PIN = 34;
const int SMOKE_THRESHOLD = 2000;

void setup() {
    Serial.begin(115200);
}

void loop() {
    int smokeValue = analogRead(MQ2_PIN);

    Serial.print("Smoke Value: ");
    Serial.println(smokeValue);

    if (smokeValue > SMOKE_THRESHOLD) {
        Serial.println("⚠️ ตรวจพบควันหรือแก๊ส!");
    }

    delay(1000);
}
