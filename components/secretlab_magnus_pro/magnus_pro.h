#pragma once

#include <cinttypes>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome
{
namespace secretlab
{
class SecretLabMagnusPro : public Component
{
public:
	void setup() override;
	void loop() override;
	void dump_config() override;

	void set_controller(uart::UARTComponent *controller) { this->controller_ = controller; }
	void set_remote(uart::UARTComponent *remote) { this->remote_ = remote; }
	void set_switch(InternalGPIOPin *pin) { switch_ = pin; }
	void switch_intr();

protected:
	void recv_controller();
	void recv_remote();

	void send_controller();
	void send_remote();

	void process_controller();
	void process_remote(uint8_t unk, uint8_t keys);

	uart::UARTComponent *controller_ = 0;
	uint8_t controller_buf_[6] = {0, 0, 0, 0, 0, 0};
	uint8_t controller_seg_[3] = {0, 0, 0};
	uint8_t controller_leds_ = 0;

	uart::UARTComponent *remote_ = 0;
	bool is_remote_on_ = false;
	uint8_t last_unk_ = 0, last_keys_ = 0;

	InternalGPIOPin *switch_ = 0;

	float height_ = 0.0;
	float set_height_ = 0.0;
	int set_height_ctr_ = 0;
	int set_height_fast_limit_ = 4;
	int set_height_slow_skip_ = 10;
};
} // namespace secretlab
} // namespace esphome