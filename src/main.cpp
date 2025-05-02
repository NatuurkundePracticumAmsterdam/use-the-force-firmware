#include <algorithm>
#include <string>
#include <stdint.h>

#include "HardwareSerial.h"
#include <Arduino.h>
#include "esp32-hal-timer.h"

#include "LoadCell.h"
#include "Motor.h"
#include "Poll.h"
#include "interface.h"

#define CPU_FREQ_KHZ 8000

#ifndef CHECK_INTERVAL_MULT
#define CHECK_INTERVAL_MULT 1
#endif

#ifndef SERIAL_TIMEOUT
#define SERIAL_TIMEOUT 20
#endif

#ifndef LOOP_DELAY
#define LOOP_DELAY 50
#endif

Motor motor;
LoadCell lc;
Interface interface;
hw_timer_t *timer0 = NULL, *timer1 = NULL;
bool should_poll = false;

/* from serial */
void do_cmd(const std::string& cmd);
void IRAM_ATTR timer0_isr();

void IRAM_ATTR timer1_isr() { should_poll = true; }

static void poll_lc() {
  should_poll = false;
  poll_lc_active();
}

void setup() {
  interface.setup();
  Serial.setTimeout(SERIAL_TIMEOUT); // 20ms timeout for serial read
  Serial.begin(115200);
  motor.begin();
  lc.begin();
  /* using freq in khz we can mul target millis by 10 and fit both vals in arr and psc */
  timer0 = timerBegin(0, CPU_FREQ_KHZ, true);
  timerAttachInterrupt(timer0, &timer0_isr, true);

  timer1 = timerBegin(1, CPU_FREQ_KHZ*CHECK_INTERVAL_MULT, true);
  timerAttachInterrupt(timer1, &timer1_isr, true);
  timerAlarmWrite(timer1, 100*10, true);
  timerAlarmEnable(timer1);
  poll_lc();
  delay(500);
  interface.update_force_zero();
  interface.clear();
}

void loop() {
  if (Serial.available()) {
    std::string cmd = Serial.readString().c_str();
    std::transform(cmd.begin()+1, cmd.begin()+3, cmd.begin()+1, ::toupper);
    do_cmd(cmd);
  }
  if (should_poll)
    poll_lc();
    interface.interface_update_interval--;
    if (interface.interface_update_interval == 0) {
      interface.interface_update_interval = INTERFACE_READ_LOOPS;
      interface.loop();
  }
  
  delay(LOOP_DELAY);
}
