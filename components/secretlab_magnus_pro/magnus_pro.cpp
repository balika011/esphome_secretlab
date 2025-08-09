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

uint8_t alpha_7seg[] = { 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x3d, 0x74, 0x30, 0x1e, 0x75, 0x38, 0x15, 0x37, 0x3f, 0x73, 0x67, 0x33, 0x6d, 0x78, 0x3e, 0x2e, 0x2a, 0x76, 0x6e, 0x4b };
uint8_t num_7seg[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

static char _7seg_to_char(uint8_t seg)
{
  if (seg == 0x00)
    return ' ';
  
  if (seg == 0x40)
    return '-';

  for (int i = 0; i < sizeof(alpha_7seg); i++)
    if (alpha_7seg[i] == (seg & 0x7f))
      return 'A' + i;

  for (int i = 0; i < sizeof(num_7seg); i++)
    if (num_7seg[i] == (seg & 0x7f))
      return '0' + i;

  ESP_LOGE(TAG, "unknown character: %02x", seg);

  return '?';
}

void SecretLabMagnusPro::process_controller(uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t leds)
{
  if (this->last_seg1_ == seg1 && this->last_seg2_ == seg2 && this->last_seg3_ == seg3 && this->last_leds_ == leds)
    return;

  this->last_seg1_ = seg1;
  this->last_seg2_ = seg2;
  this->last_seg3_ = seg3;
  this->last_leds_ = leds;

  ESP_LOGD(TAG, "controller: %c %c %c %02x", _7seg_to_char(seg1), _7seg_to_char(seg2), _7seg_to_char(seg3), leds);
}

void SecretLabMagnusPro::process_remote(uint8_t unk, uint8_t keys)
{
  if (this->last_unk_ == unk && this->last_keys_ == keys)
    return;

  this->last_unk_ = unk;
  this->last_keys_ = keys;

  ESP_LOGD(TAG, "remote: %02x %02x", unk, keys);
}

} //namespace secretlab
} //namespace esphome