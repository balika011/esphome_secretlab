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

  void set_controller(uart::UARTDevice *controller) { }
  void set_remote(uart::UARTDevice *remote) { }

  protected:
};

}  // namespace secretlab
}  // namespace esphome