/*
 *  Wrappers for some of the nvs funtions
 *  TODO: Better error handling
 */

#ifndef NVS_UTIL_H
#define NVS_UTIL_H

#include "esp_err.h"
#include "nvs.h"
#include "nvs_handle.hpp"

#include <memory>

namespace nvs_util {
void init();

std::unique_ptr<nvs::NVSHandle> get_handle();

// returns true if kv-pair was found
template <typename T>
bool read(const char* key, T& value, nvs::NVSHandle* handle) {
  esp_err_t err = handle->get_item(key, value);
  if (err != ESP_ERR_NVS_NOT_FOUND && err != ESP_OK) {
    ESP_ERROR_CHECK(err); // this is only for actual read errors, not should init to zero
  }
  return err != ESP_ERR_NVS_NOT_FOUND;
}

template <typename T>
void write(const char* key, T& value, nvs::NVSHandle* handle) {
  esp_err_t err = handle->set_item(key, value);
  ESP_ERROR_CHECK(err);
}
};

#endif // !NVS_UTIL_H
