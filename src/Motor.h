#ifndef MOTOR_H
#define MOTOR_H

#include <cstdint>

class Motor {
public:
  Motor() = default;
  virtual ~Motor() = default;

  void begin();

  void set_pos_mm(uint8_t mm);
  void set_vel_mms(uint8_t mms);
  void home();
  void abort();

  uint8_t get_pos_mm() const { return pos_mm_; }
  uint8_t get_vel_mms() const { return vel_mms_; }

private:
  const uint8_t max_vel_mms_ = 120;
  const uint8_t max_pos_mm_ = 46;  /* 47 mm hits the physical top */
  uint8_t vel_mms_ = 120;
  uint8_t pos_mm_ = -1;

  const uint8_t pin_rx_ = 15;
  const uint8_t pin_tx_ = 13;
};

#endif // !MOTOR_H
