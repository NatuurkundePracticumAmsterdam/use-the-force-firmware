#include "LoadCell.h"
#include "Motor.h"

#include "HardwareSerial.h"
#include <Arduino.h>
#include "HX711.h"
#include "esp32-hal-timer.h"

#include <string>

void do_cmd(const std::string& cmd);
void IRAM_ATTR timer_cb();

#define CPU_FREQ_KHZ 8000

Motor motor;
LoadCell lc;
hw_timer_t *timer = NULL;

void setup() {
  Serial.begin(115200);
  motor.begin();
  lc.begin();
  /* using freq in khz we can mul target millis by 10 and fit both vals in arr and psc */
  timer = timerBegin(0, CPU_FREQ_KHZ, true);
  timerAttachInterrupt(timer, &timer_cb, true);
}

void loop() {
  if (Serial.available()) {
    std::string cmd = Serial.readString().c_str();
    do_cmd(cmd);
  }
  delay(50);
}
