#include "LoadCell.h"
#include "Motor.h"

#include "esp32-hal-timer.h"
#include "HardwareSerial.h"
#include "esp_timer.h"

#include <cstring>
#include <vector>
#include <string>
#include <cmath>

#ifndef NUM_READS
#define NUM_READS 1
#endif

/* important: cmds must be sorted by number of args, ascendingly */
#define COMMANDS                                  \
  /* 0 Arguments */                               \
  X(AB)  /* abort continuous read */              \
  X(ST)  /* stop motor */                         \
  X(GP)  /* get pos in mm */                      \
  X(GV)  /* get velocity in mm/s */               \
  X(SR)  /* single read */                        \
  X(ID)  /* get motor id */                       \
  X(GM)  /* get read mode */                      \
  X(TM)  /* toggle read mode */                   \
  X(HM)  /* home stage */                         \
  X(TR)  /* tare load cell */                     \
  X(CL)  /* calibrate load cell */                \
  X(SC)  /* save loadcell config to flash */      \
  X(SF)  /* set calib force */                    \
  /* 1 Argument */                                \
  X(SP)  /* set pos in mm */                      \
  X(SV)  /* set velocity in mm/s */               \
  /* 2 Arguments */                               \
  X(CR)  /* continuous read for n milliseconds */ \

enum commands {
  #define X(cmd) cmd,
    COMMANDS
  #undef X
};

union arg {
  int32_t i;
  float f;
};

extern hw_timer_t *timer0;
uint32_t timer0_isr_iter = 0;
uint64_t timer_start_us = 0;

int8_t current_cmd;
union arg current_args[2];

extern Motor motor;
extern LoadCell lc;

static bool correct_num_args(uint8_t num_args) {
  if (current_cmd < SF && num_args == 0)
    return true;
  if (current_cmd < CR && num_args == 1)
    return true;
  if (current_cmd == CR && num_args == 2)
    return true;
  return false;
}

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
  if (!correct_num_args(args.size()))
    current_cmd = -1;
}

void IRAM_ATTR timer0_isr() {
  double val = lc.read(NUM_READS);
  uint32_t timediff_ms = (esp_timer_get_time() - timer_start_us) / 1000;
  Serial.printf("[TIME;VALUE]: %u;%f\n", timediff_ms, val);

  timer0_isr_iter--;
  if (timer0_isr_iter <= 0) {
    timerStop(timer0);
  }
}

void do_cmd(const std::string& cmd) {
  parse_cmd(cmd);
  if (current_cmd == (uint32_t) -1) {
    Serial.println("[ERROR]: invalid command");
    return;
  }
  switch (current_cmd) {
    case AB:
      timer0_isr_iter = 0;
      break;
    case ST:
      motor.abort();
      Serial.printf("[INFO]: stopping motor\n");
      break;
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
    case GV:
      Serial.printf("[VEL]: %u\n", motor.get_vel_mms());
      break;
    case HM:
      motor.home();
      Serial.printf("[INFO]: homing\n");
      break;
    case GM:
      Serial.printf("[INFO]: current mode is %s\n", lc.get_mode() ? "cal" : "raw");
      break;
    case TM:
      Serial.printf("[INFO]: toggling mode\n");
      lc.toggle_mode();
      break;
    case SR:
      Serial.printf("[VALUE]: %f\n", lc.read());
      break;
    case CR:
      timer0_isr_iter = current_args[0].i;
      timer_start_us = esp_timer_get_time();
      timerWrite(timer0, 0);
      timerAlarmWrite(timer0, std::round(current_args[1].i * 10), true);
      timerAlarmEnable(timer0);
      timerStart(timer0);
      break;
    case TR:
      Serial.printf("[INFO]: tare loadcell\n");
      lc.tare();
      break;
    case CL:
      Serial.printf("[INFO]: ready for calibration\n");
      lc.zero();
      break;
    case SF:
      Serial.printf("[INFO]: calibrating loadcell with value: %f\n", current_args[0].f);
      lc.set_slope(current_args[0].f);
      break;
    case SC:
      Serial.printf("[INFO]: writing load cell config to flash\n");
      lc.save_state();
      break;
  }
  return;
}
