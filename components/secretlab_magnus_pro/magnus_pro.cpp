#include "magnus_pro.h"
#include "esphome/core/log.h"

namespace esphome {
namespace secretlab {

static const char *const TAG = "secretlab.magnus_pro";

void SecretLabMagnusPro::setup()
{
}

void SecretLabMagnusPro::loop()
{
  ESP_LOGD(TAG, "SecretLabMagnusPro:loop %x %x", this->controller_, this->controller_->available());
/*  while (this->controller_->available())
  {
    uint8_t byte;
    this->controller_->read_byte(&byte);
    ESP_LOGD(TAG, "controller: %02x", byte);
  }

  while (this->remote_->available())
  {
    uint8_t byte;
    this->remote_->read_byte(&byte);
    ESP_LOGD(TAG, "remote: %02x", byte);
  }
    */
}

void SecretLabMagnusPro::dump_config()
{
}

} //namespace secretlab
} //namespace esphome