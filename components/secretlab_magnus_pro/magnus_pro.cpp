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
  ESP_LOGD(TAG, "SecretLabMagnusPro:loop %x %x", this->controller_->available(), this->remote_->available());

  recv_controller();

  recv_remote();
}

void SecretLabMagnusPro::dump_config()
{
}

void SecretLabMagnusPro::recv_controller()
{
  if (this->controller_->available() < 6)
  {
    ESP_LOGD(TAG, "controller: available: %02x", this->controller_->available());
    return;
  }

  uint8_t byte;
  this->controller_->read_byte(&byte);
  if (byte != 0x5a)
  {
    ESP_LOGD(TAG, "controller: %02x != 5a", byte);
    return;
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