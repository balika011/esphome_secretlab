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

  recv_remote();
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
  if (byte != 0x5a)
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
    ESP_LOGD(TAG, "controller: Invalid checksum! %02x != %02x", checksum, msg[4]);
    return;
  }

  process_controller(msg[0], msg[1], msg[2], msg[3]);
}

void SecretLabMagnusPro::recv_remote()
{
  if (this->remote_->available() < 5)
    return;

  uint8_t byte;
  this->remote_->read_byte(&byte);
  if (byte != 0xa5)
  {
    ESP_LOGD(TAG, "remote: %02x != a5", byte);
    return;
  }

  uint8_t msg[4];
  this->remote_->read_array(msg, sizeof(msg));
  uint8_t checksum = 0;
  for (int i = 0; i < sizeof(msg) - 1; i++)
    checksum += msg[i];

  if (checksum != msg[3])
  {
    ESP_LOGD(TAG, "remote: Invalid checksum! %02x != %02x", checksum, msg[3]);
    return;
  }

  if (msg[1] != (uint8_t) ~msg[2])
  {
    ESP_LOGD(TAG, "controller: Keymap not match negated! %02x != %02x", msg[1], (uint8_t) ~msg[2]);
    return;
  }

  process_remote(msg[0], msg[1]);
}

void SecretLabMagnusPro::process_controller(uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t leds)
{
  ESP_LOGD(TAG, "controller: %02x %02x %02x %02x", seg1, seg2, seg3, leds);
}

void SecretLabMagnusPro::process_remote(uint8_t unk, uint8_t keys)
{
  ESP_LOGD(TAG, "remote: %02x %02x", unk, keys);
}

} //namespace secretlab
} //namespace esphome