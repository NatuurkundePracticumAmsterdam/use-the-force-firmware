/*
 *  A simple interface to the HX711 API that stores its state in NVS
 */

#ifndef LOADCELL_H
#define LOADCELL_H
#include "HX711.h"

#define MODE_RAW false
#define MODE_CAL true

class LoadCell {
  public:
  void begin(uint8_t dout_pin = 15, uint8_t sck_pin = 13);
  void save_state();
  void save_max_counts(int32_t max_counts);
  void save_max_counts_zero(int32_t max_counts_zero);
  int32_t get_max_counts();
  int32_t get_max_counts_zero();

  void tare();
  void zero(); /* call before placing calibration weight and setting slope */
  void set_slope(double newtons); /* call after zeroing and placing calibration weight */
  double read(uint8_t i = 1);
  long quick_read();

  bool is_calibrated() const { return calibrated_; }
  bool get_mode() const { return mode_; }
  bool get_offset() const { return offset_; }
  void toggle_mode() { mode_ = !mode_; }
  int32_t max_counts = INIT_MAX_COUNTS;
  int32_t max_counts_zero = INIT_MAX_COUNTS_ZERO;

  private:
  uint32_t offset_ = 0, slope_ = 1;
  HX711    loadcell_;
  bool     calibrated_ = false;
  bool     mode_ = calibrated_;
};

#endif // !LOADCELL_H
