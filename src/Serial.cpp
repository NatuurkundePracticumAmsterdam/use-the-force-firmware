#include "LoadCell.h"
#include "Motor.h"

#include "esp32-hal-timer.h"
#include "HardwareSerial.h"
#include "esp_timer.h"

#include <cstring>
#include <vector>
#include <string>
#include <cmath>

#define COMMANDS                                  \
  X(SP)  /* set pos in mm */                      \
  X(GP)  /* get pos in mm */                      \
  X(SV)  /* set velocity in mm/s */               \
  X(GV)  /* get velocity in mm/s */               \
  X(CR)  /* continuous read for n milliseconds */ \
  X(SR)  /* single read */                        \
  X(ID)  /* get motor id */                       \
  X(GM)  /* get read mode */                      \
  X(TM)  /* toggle read mode */                   \
  X(HM)  /* home stage */                         \
  X(TR)  /* tare load cell */                     \
  X(CL)  /* calibrate load cell */                \
  X(SF)  /* set calib force */                    \

enum commands {
  #define X(cmd) cmd,
    COMMANDS
  #undef X
};

union arg {
  int32_t i;
  float f;
};

extern hw_timer_t *timer;
uint32_t timer_cb_iter = 0;
uint64_t timer_start_us = 0;

int8_t current_cmd;
union arg current_args[2];

extern Motor motor;
extern LoadCell lc;

static void get_args(const std::string& cmd, std::vector<std::string>& vec) {
  if (cmd.length() <= 4) {
    vec = {};
    return;
  }
  std::string args = cmd.substr(3, cmd.length() - 4);
  auto begin = args.begin();
  for (auto c = args.begin(); c <= args.end(); c++) {
    if (c == args.end() || *c == ',') {
      vec.push_back(std::string(begin, c));
      if (c != args.end())
        begin = c + 1;
    }
  }
}

static void parse_cmd(const std::string& cmd) {
  bool valid = true;
  valid &= cmd.front() == '#';
  valid &= cmd.back() == ';';
  valid &= cmd.length() >= 4;  /* a valid command contains at least 2 delimiters and a 2-byte opcode */

  if (!valid) {
    current_cmd = -1;
    return;
  }

  bool valid_opcode = false;
  const std::string& opcode = cmd.substr(1, 2);
  #define X(val) if (!strncmp(#val, opcode.c_str(), 2)) { \
    current_cmd = val;   \
    valid_opcode = true; \
  }
    COMMANDS
  #undef X

  if (!valid_opcode) {
    current_cmd = -1;
    return;
  }

  std::vector<std::string> args;
  get_args(cmd, args);
  if (!args.empty()) {
    switch (current_cmd) {
      case SF:
        current_args[0].f = atof(args[0].c_str());
        break;
      case CR:
        current_args[1].i = atoi(args[1].c_str());
      default:
        current_args[0].i = atoi(args[0].c_str());
        break;
    }
  }
}

void IRAM_ATTR timer_cb() {
  Serial.printf("timer: %u\n", (esp_timer_get_time() - timer_start_us) / 1000);
  // double val = lc.read(3);

  // uint64_t ms = esp_timer_get_time() - timer_start_us) / 1000;
  timer_cb_iter--;
  if (timer_cb_iter == 0) {
    timerStop(timer);
  }
}

void do_cmd(const std::string& cmd) {
  parse_cmd(cmd);
  if (current_cmd == (uint32_t) -1) {
    Serial.println("[INFO]: invalid command");
    return;
  }
  switch (current_cmd) {
    case SP:
      motor.set_pos_mm(current_args[0].i);
      Serial.printf("[INFO]: move %i\n", current_args[0].i);
      break;
    case GP:
      Serial.printf("[POS]: %u\n", motor.get_pos_mm());
      break;
    case SV:
      motor.set_vel_mms(current_args[0].i);
      Serial.printf("[INFO]: set velocity %u\n", current_args[0].i);
      break;
    case HM:
      motor.home();
      Serial.printf("[INFO]: homing\n");
      break;
    case SR:
      /* adjust readme to reflect actual behaviour */
      Serial.printf("%f\n", lc.read());
      break;
    case CR:
      timer_cb_iter = current_args[0].i;
      timer_start_us = esp_timer_get_time();
      timerAlarmWrite(timer, std::round(current_args[1].i * 10), true);
      timerAlarmEnable(timer);
      timerStart(timer);
      break;
    /* TODO: remove for release */
    case ID:
      Serial2.printf("ID\r\n");
      delay(50);
      Serial.printf("[INFO]: ID %s\n", Serial2.readString());
      break;
    default:
      break;
  }
  return;
}
