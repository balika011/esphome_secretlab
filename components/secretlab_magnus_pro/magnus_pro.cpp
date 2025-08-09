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
  recv_controller();

  if (this->remote_->available())
  {
    uint8_t byte;
    this->remote_->read_byte(&byte);
    ESP_LOGD(TAG, "remote: %02x", byte);
  }
}

void SecretLabMagnusPro::dump_config()
{
}

void SecretLabMagnusPro::recv_controller()
{
  if (this->controller_->available() < 6)
    return;

  uint8_t byte;
  this->controller_->read_byte(&byte);
  if (byte != 0x5A)
  {
    ESP_LOGD(TAG, "controller: %02x != 5a", byte);
    return;
  }

  uint8_t msg[5];
  this->controller_->read_array(msg, sizeof(msg));
  uint8_t checksum = 0;
  for (int i = 0; i < sizeof(msg) - 1; i++)
    checksum += msg[i];

  if (checksum != msg[4])
  {
    printf("Invalid checksum! %02x != %02x", checksum, msg[4]);
  }

  printf("controller: %02x %02x %02x %02x", msg[0], msg[1], msg[2], msg[3]);
}

} //namespace secretlab
} //namespace esphome