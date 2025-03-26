#include "LoadCell.h"
#include "nvs_util.h"

void LoadCell::begin(uint8_t dout_pin, uint8_t sck_pin) {
  nvs_util::init();
  auto  handle = nvs_util::get_handle();
  calibrated_ |= nvs_util::read("offset_", offset_, handle.get());
  calibrated_ |= nvs_util::read("slope_", slope_, handle.get());

  loadcell_.begin(dout_pin, sck_pin);
  loadcell_.set_offset(offset_);
  loadcell_.set_scale(slope_);
  // loadcell_.power_up();
}

void LoadCell::save_state() {
  auto handle = nvs_util::get_handle();
  nvs_util::write("offset_", offset_, handle.get()); // TODO: unique id per instance
  nvs_util::write("slope_", slope_, handle.get());
}

void LoadCell::tare() {
  loadcell_.tare();
  offset_ = loadcell_.get_offset();
}

void LoadCell::zero() {
  loadcell_.set_scale();
  loadcell_.tare();
}

void LoadCell::set_slope(double newtons) {
  loadcell_.set_scale(loadcell_.get_units(10) / newtons);
  offset_ = loadcell_.get_offset();
  slope_ = loadcell_.get_scale();
}

double LoadCell::read(uint8_t i) {
  if (mode_)
    return loadcell_.get_units(i);
  return loadcell_.read_average(i);
}
