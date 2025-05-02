#include "LoadCell.h"
#include "nvs_util.h"

void LoadCell::begin(uint8_t dout_pin, uint8_t sck_pin) {
  nvs_util::init();
  auto handle = nvs_util::get_handle();
  calibrated_ = false;
  calibrated_ |= nvs_util::read("offset_", offset_, handle.get());
  calibrated_ &= nvs_util::read("slope_", slope_, handle.get());
  mode_ = calibrated_;

  loadcell_.begin(dout_pin, sck_pin);
  loadcell_.set_offset(offset_);
  loadcell_.set_scale(slope_);
  // loadcell_.power_up();
  max_counts_zero = get_max_counts_zero();
  max_counts = get_max_counts();

}

void LoadCell::save_state() {
  auto handle = nvs_util::get_handle();
  nvs_util::write("offset_", offset_, handle.get()); // TODO: unique id per instance
  nvs_util::write("slope_", slope_, handle.get());
}

void LoadCell::save_max_counts(int32_t max_counts) {
  auto handle = nvs_util::get_handle();
  nvs_util::write("max_counts", max_counts, handle.get()); 
}

void LoadCell::save_max_counts_zero(int32_t max_counts_zero) {
  auto handle = nvs_util::get_handle();
  nvs_util::write("max_counts_zero", max_counts_zero, handle.get()); 
}

int32_t LoadCell::get_max_counts() {
  auto handle = nvs_util::get_handle();
  nvs_util::read("max_counts", max_counts, handle.get());
  return max_counts;
}

int32_t LoadCell::get_max_counts_zero() {
  auto handle = nvs_util::get_handle();
  nvs_util::read("max_counts_zero", max_counts_zero, handle.get());
  return max_counts_zero;
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

long LoadCell::quick_read() {
  return static_cast<int32_t>(loadcell_.read());
}
