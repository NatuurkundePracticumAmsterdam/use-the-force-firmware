#include "LoadCell.h"
#include "Motor.h"
#include "Poll.h"
#include "interface.h"

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

#ifndef INIT_MAX_COUNTS
#define INIT_MAX_COUNTS INT32_MAX
#endif

#ifndef INIT_MAX_COUNTS_ZERO
#define INIT_MAX_COUNTS_ZERO 0
#endif

#ifndef VERSION
#define VERSION "-1.-1.-1"
#endif

/* important: cmds must be sorted by number of args, ascendingly */
#define COMMANDS                                        \
  /* 0 Arguments */                                     \
  X(AB)  /* abort continuous read */                    \
  X(CM)  /* Count Max, set maximum force count */       \
  X(CZ)  /* Count Zero, set maximum force count zero */ \
  X(GP)  /* get pos in mm */                            \
  X(GV)  /* get velocity in mm/s */                     \
  X(HE)  /* Help command, with all commands */          \
  X(HM)  /* home stage */                               \
  X(ID)  /* get motor id */                             \
  X(SD)  /* Serial Dump, sends what is in serial rn*/   \
  X(SR)  /* single read */                              \
  X(ST)  /* stop motor */                               \
  X(TR)  /* tare force value */                         \
  X(VR)  /* get version */                              \
                                                        \
  /* 1 Argument */                                      \
  X(DC)  /* Display Command: true/false */              \
  X(SF)  /* set calib force */                          \
  X(SP)  /* set pos in mm */                            \
  X(SV)  /* set velocity in mm/s */                     \
  X(UU)  /* update unit */                              \
  X(UX)  /* update interface x offset */                \
  X(UY)  /* update interface y offset */                \
  X(UL)  /* update interface y line spacing */          \
                                                        \
  /* 2 Arguments */                                     \
  X(CR)  /* continuous read for n milliseconds */       \

enum commands {
  #define X(cmd) cmd,
    COMMANDS
  #undef X
};

union arg {
  int32_t i;
  float f;
  char s[10];
};

extern hw_timer_t *timer0;
uint32_t timer0_isr_iter = 0;
uint64_t timer_start_us = 0;
int32_t val = 0;

int8_t current_cmd;
union arg current_args[2];
bool display_cmd = true;
char false_str[10] = "false";

bool returnRead = false;
bool singleRead = false;

extern Motor motor;
extern LoadCell lc;
extern Interface interface;

static bool correct_num_args(uint8_t num_args) {
  if (current_cmd < DC && num_args == 0)
    return true;
  if (current_cmd == DC && num_args <= 1)
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

void poll_lc_active() {
  val = lc.quick_read();

  if (abs(val-lc.max_counts_zero) > abs(lc.max_counts-lc.max_counts_zero)) {
    motor.abort();
    // Serial.println("[ERROR]: strain too high, stopping motor now");
  }

  else if (returnRead) {
    if (singleRead) {
      Serial.printf("[VALUE]: %d\n", val);
      singleRead = false;
    }
    else {
      uint32_t timediff_ms = (esp_timer_get_time() - timer_start_us) / 1000;
      Serial.printf("[TIME;VALUE]: %u;%d\n", timediff_ms, val);
    }
    returnRead = false;
  }

  interface.forceVec[INTERFACE_READ_LOOPS-interface.interface_update_interval-1] = val;
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
      case UU:
        if (args[0].length() >= sizeof(current_args[0].s)) {
          Serial.println("[ERROR]: Unit string too long");
          current_cmd = -1;
          return;
        }
        strncpy(current_args[0].s, args[0].c_str(), sizeof(current_args[0].s) - 1);
        current_args[0].s[sizeof(current_args[0].s) - 1] = '\0';
        break;
      case DC:
        if (args[0] == "false" || args[0] == " false") {
          display_cmd = false;
        } else {
          display_cmd = true;
        }
        break;
      case CR:
        current_args[1].i = atoi(args[1].c_str());
      default:
        current_args[0].i = atoi(args[0].c_str());
        break;
    }
  } else if (current_cmd == DC) {
    display_cmd = true;
  }
  if (!correct_num_args(args.size()))
    current_cmd = -1;
}

void IRAM_ATTR timer0_isr() {
  returnRead = true;

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
  if (display_cmd) {
    interface.show_command(cmd);
  }
  switch (current_cmd) {
    case AB:
      timer0_isr_iter = 0;
      break;
    case ST:
      motor.abort();
      Serial.printf("[INFO]: stopping motor, needs to reHome\n");
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
    case SR:
      returnRead = true;
      singleRead = true;
      break;
    case CR:
      timer0_isr_iter = current_args[0].i;
      timer_start_us = esp_timer_get_time();
      timerWrite(timer0, 0);
      timerAlarmWrite(timer0, std::round(current_args[1].i * 10), true);
      timerAlarmEnable(timer0);
      timerStart(timer0);
      returnRead = true;
      break;
    case TR:
      Serial.printf("[INFO]: taring force\n");
      interface.update_force_zero();
      break;
    case SF:
      interface.update_force_slope(current_args[0].f);
      break;
    case CM:
      lc.max_counts = abs(val);
      lc.save_max_counts(abs(val));
      Serial.printf("[INFO]: max counts set to %d\n", lc.max_counts);
      break;
    case CZ:
      lc.max_counts_zero = val;
      lc.save_max_counts_zero(val);
      Serial.printf("[INFO]: max counts zero set to %d\n", lc.max_counts_zero);
      break;
    case VR:
      Serial.printf("[VERSION]: %s\n", VERSION);
      break;
    case UX:
      interface.update_x_offset(current_args[0].i);
      Serial.printf("[INFO]: updated x offset to %d\n", current_args[0].i);
      break;
    case UY:
      interface.update_y_offset(current_args[0].i);
      Serial.printf("[INFO]: updated y offset to %d\n", current_args[0].i);
      break;
    case UL:
      interface.update_line_height(current_args[0].i);
      Serial.printf("[INFO]: updated line y offset to %d\n", current_args[0].i);
      break;
    case UU:
      interface.update_unit(std::string(current_args[0].s));
      Serial.printf("[INFO]: updated unit to %s\n", current_args[0].s);
      break;
    case DC:
      if (display_cmd) {Serial.printf("Displaying commands\n");}
      else {Serial.printf("Not displaying commands\n");}
      break;
    case SD:
      Serial.printf("\n");
      break;
    case HE:
      std::string help_message;
      uint8_t args_count = 0;
      
      help_message += "--- 0 Arguments ---\n";
      #define X(cmd) \
          if (cmd < SF) { \
              help_message += "#" #cmd ";\n"; \
          }
      COMMANDS
      #undef X
  
      help_message += "\n--- 1 Argument ---\n";
      #define X(cmd) \
          if (cmd >= SF && cmd < CR) { \
              help_message += "#" #cmd " <arg1>;\n"; \
          }
      COMMANDS
      #undef X
  
      help_message += "\n--- 2 Arguments ---\n";
      #define X(cmd) \
          if (cmd == CR) { \
              help_message += "#" #cmd " <arg1>, <arg2>;\n"; \
          }
      COMMANDS
      #undef X
  
      Serial.printf("[COMMANDS]:\n%s", help_message.c_str());
      break;
  }
  return;
}
