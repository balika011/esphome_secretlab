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
  void set_remote(uart::UARTComponent *remote) { this->remote_ = remote; }

 protected:
  void recv_controller();
  void recv_remote();

  void process_controller(uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t leds);
  void process_remote(uint8_t unk, uint8_t keys);

  uart::UARTComponent *controller_ = 0;
  uart::UARTComponent *remote_ = 0;
};

}  // namespace secretlab
}  // namespace esphome