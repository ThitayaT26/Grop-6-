# Grop-6/Smart Haven
# Team Smart Haven

| ชื่อสมาชิก | Role |
| --- | --- |
| ทิตยา แต้มสาระ | Embedded / IoT Developer |
| ปวรวรรณ ย่องใย | Product / UX |
| พิชชาภา คงมหาพฤกษ์ | Project Manager / Scrum Lead |
| ศิวัฒน์ ศิริสวัสดิ์ | Product / UX |

## Integration Map (แผนภาพการเชื่อมระบบ)
| ส่วน | คำตอบของทีม |
| --- | --- |
| Input คืออะไร | ค่าควัน/อุณหภูมิ/เปลวไฟจากเซ็นเซอร์ (MQ-2/135, Flame sensor, DS18B20) |
| Component 1 | ESP32 (อ่านค่าเซ็นเซอร์, ประมวลผล, สั่งงาน)) |
| Component 2 | Relay + ปั๊มโฟม (หรือ solenoid valve) สำหรับปล่อยโฟมดับเพลิง |
| Component 3 | Buzzer + LED แจ้งเตือนภายในพื้นที่ |
| Component 4 | MQTT / Firebase + Dashboard (แจ้งเตือนผู้ใช้ผ่านมือถือ) |
| Output คืออะไร | ปล่อยโฟมดับเพลิง, เสียง/แสงแจ้งเตือน, ผู้ใช้เห็นสถานะบนมือถือ |

## Integration Map ของทีม (เขียนเป็น flow สั้น ๆ)
```
เซ็นเซอร์ (ควัน/เปลวไฟ/อุณหภูมิ)
         ↓
       ESP32
    ↓         ↓
Buzzer/LED   ตรวจสอบค่าเกิน threshold
    ↓         ↓
แจ้งเตือน    สั่ง Relay เปิดปั๊มโฟม
ในพื้นที่         ↓
            ปล่อยโฟมดับเพลิง
                ↓
            ส่งข้อมูลผ่าน MQTT/Firebase
                ↓
         Dashboard แสดงสถานะ
                ↓
         แจ้งเตือนผู้ใช้ทางมือถือ
```
## Scope Cut Table (ตัด scope อย่างเป็นทางการ)
| Must Finish for Demo | Can Demo with Workaround | Cut for Sprint 3 |
| --- | --- | --- |
| ESP32 อ่านค่า MQ-2 (ควัน) และ Flame Sensor ได้ | ใช้ Serial Monitor แสดงค่าแทน Dashboard | ระบบปล่อยโฟมจริง (ปั๊ม + Relay) |
| ตั้งค่า threshold และแจ้งเตือน Buzzer/LED เมื่อค่าเกิน | ใช้ MQTT Explorer ดูข้อมูลแทนการแจ้งเตือนมือถือ | ระบบแจ้งเตือน LINE / Push Notification |
| ส่งข้อมูลผ่าน MQTT ไปยัง Broker ได้ | ใช้ Video backup สาธิตการแจ้งเตือน | AI วิเคราะห์ระดับความรุนแรงของเพลิงไหม้ |

##  Prototype: ระบบตอนนี้ทำอะไรได้บ้าง

Prototype v1 ของทีม Smart Haven สามารถจำลองการตรวจจับควัน/เพลิงไหม้และแจ้งเตือนภายในพื้นที่ได้ในระดับเบื้องต้น โดยมีความสามารถดังนี้

* เซ็นเซอร์ MQ-2 (ควัน/แก๊ส) และ Flame Sensor สามารถอ่านค่าและแสดงผลทาง Serial Monitor ได้
* ระบบสามารถตั้งค่า threshold และแจ้งเตือนด้วย Buzzer/LED เมื่อค่าควันหรือเปลวไฟเกินค่าที่กำหนด
*ผู้ใช้สามารถปรับค่า threshold ได้ผ่านการแก้ไขโค้ด (ใช้ Serial Monitor ในการดูค่าปัจจุบัน)
*ESP32 สามารถส่งข้อมูลผ่าน MQTT ไปยัง Broker (MQTT Explorer ใช้ดูข้อมูลได้)
*มีการออกแบบ User Flow และ Integration Map สำหรับอธิบายการทำงานของระบบ
*มี GitHub Repository สำหรับรวม code, evidence, README และเอกสารของทีม
*มีการบันทึกวิดีโอสาธิตการทำงานของเซ็นเซอร์และการแจ้งเตือน (ใช้เป็น workaround กรณียังเชื่อม Dashboard ไม่ได้)
## สิ่งที่ยังทำไม่ได้ใน Prototype ตอนนี้

*Hardware (ESP32 + เซ็นเซอร์) ยังไม่เชื่อมกับ Dashboard แบบ Real-time
*MQTT / Firebase ยังทำงานไม่สมบูรณ์ (รับข้อมูลจาก ESP32 ได้ แต่ยังไม่แสดงผลบน UI)
*ระบบปล่อยโฟม (Relay + ปั๊ม) ยังทำงานได้ไม่เสถียร (ปัจจุบันใช้ LED แทนการทำงาน)
*ระบบ Login และ Push Notification ยังไม่ได้พัฒนา
*Demo บางส่วนยังใช้ Figma Prototype และ Screen Recording เป็น workaround
## Known Issues 
Prototype v1 ยังเชื่อม Hardware (ESP32 + เซ็นเซอร์) และ Dashboard แบบ Real-time ได้ไม่สมบูรณ์ จึงยังใช้ Serial Monitor, MQTT Explorer และ Demo Video เป็น workaround บางส่วน


## Sprint 4 Test Plan
| หัวข้อ | คำตอบ |
| --- | --- |
| ผู้ใช้ที่จะทดสอบ | เจ้าของบ้านหรือผู้ดูแลอาคาร 2-3 คน |
| Task ที่ให้ลองทำ | ทำให้เซ็นเซอร์ตรวจจับควัน (ใช้ธูปหรือไฟแช็ก) แล้วสังเกตการแจ้งเตือน (Buzzer/LED) และดูข้อมูลบน Serial Monitor / MQTT Explorer |
| สิ่งที่จะสังเกต | ผู้ใช้เข้าใจการแจ้งเตือนหรือไม่, ระบบตอบสนองเร็วแค่ไหน, มีการแจ้งเตือนปลอม (false positive) หรือไม่ |
| วิธีเก็บ feedback | แบบสอบถาม + สัมภาษณ์ |
| ตัวชี้วัดเบื้องต้น | ความถูกต้องในการตรวจจับ, ความรวดเร็วในการแจ้งเตือน, ความพึงพอใจของผู้ใช้ |
| สิ่งที่ต้องเตรียมก่อน Test | Prototype Hardware (ESP32 + เซ็นเซอร์ + Buzzer/LED), Demo Video, Feedback Form, Serial Monitor หรือ MQTT Explorer |

## Build Log
| รายการ | คำตอบ |
| --- | --- |
| สิ่งที่ทำเสร็จจริง 3 อันดับแรก | 1) ESP32 อ่านค่า MQ-2 และ Flame Sensor ได้ 2) Buzzer/LED แจ้งเตือนเมื่อค่าเกิน threshold 3) GitHub Evidence และ Integration Map |
| สิ่งที่ยังไม่เสร็จ | เชื่อม Hardware กับ Dashboard แบบ Real-time (MQTT/Firebase ยังไม่สมบูรณ์) |
| สิ่งที่ตัดออกจาก Sprint 3 | ระบบปล่อยโฟมจริง, Push Notification, ระบบ Login |
| สิ่งที่ใช้ workaround | ใช้ Serial Monitor, MQTT Explorer และ Demo Video แทน Dashboard |
| blocker สำคัญที่เจอ | ESP32 เชื่อม MQTT ไม่เสถียร, ค่า MQ-2 baseline เปลี่ยนแปลงตามสภาพแวดล้อม |
| วิธีแก้หรือแผนรับมือ | ใช้ MQTT broker local, ปรับ rolling average ใน code, ใช้เงื่อนไข AND (ควัน + เปลวไฟ) เพื่อลด false positive |








