#include "Motor.h"

#include "HardwareSerial.h"
#include <Arduino.h>
#include "HX711.h"

#include <string>

#define PIN_SDA 15
#define PIN_SCL 13

void do_cmd(const std::string& cmd);

HX711 lc;
Motor motor;

void setup() {
  Serial.begin(115200);
  lc.begin(PIN_SDA, PIN_SCL);
  motor.begin();
}

void loop() {
  if (Serial.available()) {
    std::string cmd = Serial.readString().c_str();
    do_cmd(cmd);
  }
  delay(50);
}
