#include "nvs_util.h"
#include "nvs_flash.h"

void nvs_util::init() {
  static bool done;
  if (done) {
    return;
  }
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  done = true;
}

std::unique_ptr<nvs::NVSHandle> nvs_util::get_handle() {
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
  ESP_ERROR_CHECK(err);
  return handle;
}
