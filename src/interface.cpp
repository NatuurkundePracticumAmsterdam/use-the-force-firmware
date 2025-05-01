#include <TFT_eSPI.h>
#include <string>
#include "interface.h"
#include "nvs_util.h"

// Initialize the display
TFT_eSPI tft = TFT_eSPI();

void Interface::setup() {
  tft.init();
  tft.setRotation(3); // Adjust rotation if needed
  tft.fillScreen(backgroundClr);
  tft.setTextColor(textClr, backgroundClr);
  
  tft.setTextFont(FONT_TYPE);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  auto handle = nvs_util::get_handle();
  if (nvs_util::read("force_slope", masked_force_slope, handle.get())) {
    memcpy(&force_slope, &masked_force_slope, sizeof(force_slope));
  } else {
    force_slope = 1.0f;
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
  std::string force_str = std::to_string(force);
  if (force_str.length() < sizeof(current_force)) {
    force_str.append(sizeof(current_force) - force_str.length(), ' ');
  }
  strncpy(current_force, force_str.c_str(), sizeof(current_force) - 1);
  current_force[sizeof(current_force) - 1] = '\0'; // Ensure null termination

  float corrected_force_value = (force - force_zero) * force_slope;
  std::string corrected_force = std::to_string(corrected_force_value);

  // Remove trailing zeros after the decimal point
  corrected_force.erase(corrected_force.find_last_not_of('0') + 1);
  if (corrected_force.back() == '.') {
    corrected_force.pop_back(); // Remove the trailing decimal point if present
  }

  // Ensure unit is appended or replaces the last part if no room
  if (corrected_force.length() + unit.length() > sizeof(corrected_force_char) - 9) {
    corrected_force = corrected_force.substr(0, sizeof(corrected_force_char) - 9 - unit.length());
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
  unit = new_unit;
}

void Interface::update_force_zero() {
  force_zero = force;
}

void Interface::update_force_slope(const float actual) {
  int32_t value = force - force_zero;
  if (value == 0) {
    force_slope = 1.0f;
    return;
  }
  force_slope = actual / value;
  memcpy(&masked_force_slope, &force_slope, sizeof(force_slope));
  auto handle = nvs_util::get_handle();
  nvs_util::write("force_slope", masked_force_slope, handle.get());
}

