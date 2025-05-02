#include <TFT_eSPI.h>
#include <string>
#include "interface.h"
#include "nvs_util.h"

// Initialize the display
TFT_eSPI tft = TFT_eSPI();

void Interface::setup() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(backgroundClr);
  tft.setTextColor(textClr, backgroundClr);
  
  tft.setTextFont(FONT_TYPE);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  auto handle = nvs_util::get_handle();

  //Load saved force_slope
  if (nvs_util::read("force_slope", masked_force_slope, handle.get())) {
    memcpy(&force_slope, &masked_force_slope, sizeof(force_slope));
  }

  // Load saved force_zero
  if (nvs_util::read("force_zero", masked_force_zero, handle.get())) {
    memcpy(&force_zero, &masked_force_zero, sizeof(force_zero));
  }
  
  // Load saved unit
  for (int i = 0; i < INTERFACE_MAX_UNIT_LENGTH; i++) {
    char key[INTERFACE_MAX_UNIT_LENGTH-1];
    snprintf(key, sizeof(key), "unit_%d", i);
    if (nvs_util::read(key, masked_unit[i], handle.get())) {
      if (masked_unit[i] != INTERFACE_UNIT_PADDING) {
        unit += static_cast<char>(masked_unit[i]);
      }
    }
  }
  if (unit.length()==0) {
    unit = " mN";
  }

  tft.setTextSize(4);
  tft.setCursor(x_offset+60, y_offset);
  tft.printf("o/");
  tft.setTextSize(FONT_SIZE);
}

void Interface::clear(){
  tft.fillScreen(backgroundClr);
}

void Interface::update_force_display() {
  avgForce = static_cast<int32_t>(std::round(std::accumulate(forceVec.begin(), forceVec.end(), 0.0) / INTERFACE_READ_LOOPS));
  std::string force_str = std::to_string(avgForce);
  if (force_str.length() < sizeof(current_force)) {
    force_str.append(sizeof(current_force) - force_str.length(), ' ');
  }
  strncpy(current_force, force_str.c_str(), sizeof(current_force) - 1);
  current_force[sizeof(current_force) - 1] = '\0'; // Ensure null termination

  float corrected_force_value = (avgForce - force_zero) * force_slope;
  std::string corrected_force = std::to_string(corrected_force_value);

  // Remove trailing zeros after the decimal point
  corrected_force.erase(corrected_force.find_last_not_of('0') + 1);
  if (corrected_force.back() == '.') {
    corrected_force.pop_back(); // Remove the trailing decimal point if present
  }

  // Ensure unit is appended or replaces the last part if no room
  if (corrected_force.length() + unit.length() > sizeof(corrected_force_char) - 6) {  // Changed from 9 to 6 to guarantee space for -9999
    corrected_force = corrected_force.substr(0, sizeof(corrected_force_char) - 6 - unit.length());
  }
  corrected_force += unit;

  // Fill the rest of the characters with spaces
  if (corrected_force.length() < sizeof(corrected_force_char) - 1) {
    corrected_force.append(sizeof(corrected_force_char) - 1 - corrected_force.length(), ' ');
  }
  
  strncpy(corrected_force_char, corrected_force.c_str(), sizeof(corrected_force_char) - 1);
  corrected_force_char[sizeof(corrected_force_char) - 1] = '\0'; // Ensure null termination

  tft.setCursor(x_offset, y_offset);
  tft.printf("Reading:");
  tft.setCursor(x_offset, y_offset + line_height);
  tft.printf("%s", current_force);
  tft.setCursor(x_offset, y_offset + 2 * line_height);
  tft.printf("%s", corrected_force_char);
}

void Interface::show_command(const std::string& command) {
  strncpy(last_command, command.c_str(), sizeof(last_command) - 1);
  last_command[sizeof(last_command) - 1] = '\0';

  command_display_time = millis();
  command_used = true;

  tft.fillScreen(backgroundClr);
  tft.setCursor(x_offset, y_offset);
  tft.printf("Command:");
  tft.setCursor(x_offset, y_offset + line_height);
  tft.printf("%s", last_command);
}

void Interface::loop() {
  if (command_used && (millis() - command_display_time > COMMAND_DISPLAY_DURATION)) {
    tft.fillRect(x_offset, y_offset, TFT_WIDTH, line_height * 2, backgroundClr);
    command_used = false;
  }
  if (!command_used) {
    update_force_display();
  }
}

void Interface::update_x_offset(const int16_t x) {
  x_offset = x;
}

void Interface::update_y_offset(const int16_t y) {
  y_offset = y;
}

void Interface::update_line_height(const int16_t height) {
  line_height = height;
}

void Interface::update_unit(const std::string& new_unit) {
  // Truncate unit if too long
  unit = new_unit.substr(0, INTERFACE_MAX_UNIT_LENGTH);
  
  // Get NVS handle
  auto handle = nvs_util::get_handle();
  
  // Store each character, pad with spaces
  for (int i = 0; i < INTERFACE_MAX_UNIT_LENGTH; i++) {
    char key[INTERFACE_MAX_UNIT_LENGTH-1];
    snprintf(key, sizeof(key), "unit_%d", i);
    
    if (i < unit.length()) {
      masked_unit[i] = static_cast<uint8_t>(unit[i]);
    } else {
      masked_unit[i] = INTERFACE_UNIT_PADDING;
    }
    
    nvs_util::write(key, masked_unit[i], handle.get());
  }
}

void Interface::update_force_zero() {
  force_zero = avgForce;
  memcpy(&masked_force_zero, &force_zero, sizeof(force_zero));
  auto handle = nvs_util::get_handle();
  nvs_util::write("force_zero", masked_force_zero, handle.get());
}

void Interface::update_force_slope(const float actual) {
  int32_t value = avgForce - force_zero;
  if (value == 0) {
    force_slope = 1.0f;
    return;
  }
  force_slope = actual / value;
  memcpy(&masked_force_slope, &force_slope, sizeof(force_slope));
  auto handle = nvs_util::get_handle();
  nvs_util::write("force_slope", masked_force_slope, handle.get());
  Serial.printf("[INFO]: calibrated force slope to: %f\n", force_slope);
}
