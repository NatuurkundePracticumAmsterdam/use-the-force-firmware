#include "Motor.h"

#include "HardwareSerial.h"

void Motor::begin() {
  Serial2.begin(9600, SERIAL_8N1, pin_rx_, pin_tx_);
  delay(200);
  Serial2.print("ID=123\r\n");
}

void Motor::set_pos_mm(uint8_t mm) {
  if (pos_mm_ == (uint8_t) -1) {
    Serial.println("[INFO]: cannot move before homing");
    return;
  }
  if (mm > max_pos_mm_) {
    Serial.println("[INFO]: requested pos is outside of valid range");
    return;
  }
  Serial2.printf("ID123:X%u\r\n", mm);
  pos_mm_ = mm;
}

void Motor::set_vel_mms(uint8_t mms) {
  if (mms > max_vel_mms_) {
    Serial.println("[INFO]: requested velocity is outside of valid range");
    return;
  }
  Serial2.printf("ID123:F%u\r\n", mms);
  vel_mms_ = mms;
}

void Motor::home() {
  Serial2.printf("ID123Z\r\n");
  pos_mm_ = 0;
  set_pos_mm(max_pos_mm_);
}
