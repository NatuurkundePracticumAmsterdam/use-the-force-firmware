#include "HardwareSerial.h"
#include "Motor.h"

#include <cstring>
#include <sstream>
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

int8_t current_cmd;
union arg current_arg;
extern Motor motor;

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

  if (cmd.length() > 4) {
    const std::string& arg = cmd.substr(3);
    if (current_cmd == SF) {  /* only cmd to take a float as arg */
      current_arg.f = std::stof(arg);
    }
    else {  /* all others take int */
      current_arg.i = std::stoi(arg);
    }
  }
}

void do_cmd(const std::string& cmd) {
  String response;
  parse_cmd(cmd);
  if (current_cmd == (uint32_t) -1) {
    Serial.println("[INFO]: invalid command");
    return;
  }
  switch (current_cmd) {
    case SP:
      motor.set_pos_mm(current_arg.i);
      Serial.printf("[INFO]: move %i\n", current_arg.i);
      break;
    case SV:
      motor.set_vel_mms(current_arg.i);
      Serial.printf("[INFO]: set velocity %u\n", current_arg.i);
      break;
    case HM:
      motor.home();
      Serial.printf("[INFO]: homing\n");
      break;
    /* TODO: remove for release */
    case ID:
      Serial2.printf("ID\r\n");
      delay(50);
      response = Serial2.readString();
      Serial.printf("[INFO]: ID %s\n", response);
      break;
    default:
      break;
  }
  return;
}
