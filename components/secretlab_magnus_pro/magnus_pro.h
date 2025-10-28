#pragma once

#include <cinttypes>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome
{
namespace secretlab
{
class SecretLabMagnusPro : public sensor::Sensor, Component
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
	void process_remote();

	uart::UARTComponent *controller_ = 0;
	uint8_t controller_buf_[6] = {0, 0, 0, 0, 0, 0};
	uint8_t controller_seg_[3] = {0, 0, 0};
	uint8_t controller_leds_ = 0;

	bool is_remote_on_ = false;
	uart::UARTComponent *remote_ = 0;
	uint8_t remote_buf_[5] = {0, 0, 0, 0, 0};
	uint8_t remote_unk_ = 0, remote_keys_ = 0;

	InternalGPIOPin *switch_ = 0;

	int height_ = 0;
	int set_height_ = 0;
	int set_height_fast_limit_ = 20;
};
} // namespace secretlab
} // namespace esphome