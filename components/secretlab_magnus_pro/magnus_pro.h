#pragma once

#include <cinttypes>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace secretlab {

class SecretLabMagnusPro : public Component
{
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_controller(uart::UARTComponent *controller) { this->controller_ = controller; }
  void set_controller_key(GPIOPin *pin) { controller_key_ = pin; }
  void set_remote(uart::UARTComponent *remote) { this->remote_ = remote; }
  void set_remote_key(GPIOPin *pin) { remote_key_ = pin; }

 protected:
  void recv_controller();
  void recv_remote();

  void process_controller(uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t leds);
  void process_remote(uint8_t unk, uint8_t keys);

  uart::UARTComponent *controller_ = 0;
  GPIOPin *controller_key_ = 0;
  uart::UARTComponent *remote_ = 0;
  GPIOPin *remote_key_ = 0;
  uint8_t last_seg1_ = 0, last_seg2_ = 0, last_seg3_ = 0, last_leds_ = 0;
  uint8_t last_unk_ = 0, last_keys_ = 0;
};

}  // namespace secretlab
}  // namespace esphome