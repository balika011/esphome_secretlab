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
}

void SecretLabMagnusPro::dump_config()
{
}

void SecretLabMagnusPro::recv_controller()
{
  if (this->controller_->available())
  {
    uint8_t byte;
    this->controller_->read_byte(&byte);
    ESP_LOGD(TAG, "controller: %02x", byte);
  }
}

void SecretLabMagnusPro::recv_remote()
{
  if (this->remote_->available())
  {
    uint8_t byte;
    this->remote_->read_byte(&byte);
    ESP_LOGD(TAG, "remote: %02x", byte);
  }
}

} //namespace secretlab
} //namespace esphome