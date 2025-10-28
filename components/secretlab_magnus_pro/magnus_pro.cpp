#include "magnus_pro.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace secretlab
{
static const char *const TAG = "secretlab.magnus_pro";

enum REMOTE_KEYS
{
	KEY_S = (1 << 0),
	KEY_1 = (1 << 1),
	KEY_2 = (1 << 2),
	KEY_3 = (1 << 3),
	KEY_UP = (1 << 5),
	KEY_DOWN = (1 << 6),
};

enum REMOTE_LEDS
{
	LED_UP = (1 << 0),
	LED_DOWN = (1 << 1),
	LED_S = (1 << 3),
	LED_1 = (1 << 4),
	LED_2 = (1 << 5),
	LED_3 = (1 << 6)
};

static char _7seg_to_char(uint8_t seg, bool upper)
{
	/*
	 0
	5 1
	 6
	4 2
	 3  7
	*/
	//                                           A     B     C     D     E     F    G      H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W     X     Y     Z
	static const uint8_t alpha_7seg_upper[] = {0x77, 0x7f, 0x39, 0x3f, 0x79, 0x71, 0x3d, 0x76, 0x30, 0x1e, 0x75, 0x38, 0x15, 0x37, 0x3f, 0x73, 0x67, 0x33, 0x6d, 0x78, 0x3e, 0x2e, 0x2a, 0x76, 0x6e, 0x4b};
	static const uint8_t alpha_7seg_lower[] = {0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x3d, 0x74, 0x30, 0x1e, 0x75, 0x38, 0x15, 0x37, 0x5e, 0x73, 0x67, 0x33, 0x6d, 0x78, 0x3e, 0x2e, 0x2a, 0x76, 0x6e, 0x4b};
	static const uint8_t num_7seg[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

	seg &= 0x7f;

	if (seg == 0x00) return ' ';

	if (seg == 0x40)
		return '-';

	for (int i = 0; i < sizeof(num_7seg); i++)
		if (num_7seg[i] == seg)
			return '0' + i;

	if (upper)
	{
		for (int i = 0; i < sizeof(alpha_7seg_upper); i++)
			if (alpha_7seg_upper[i] == seg)
				return 'A' + i;
	}
	else
	{
		for (int i = 0; i < sizeof(alpha_7seg_lower); i++)
			if (alpha_7seg_lower[i] == seg)
				return 'a' + i;
	}

	ESP_LOGE(TAG, "unknown character: %02x", seg);

	return '?';
}

static void IRAM_ATTR _switch_intr(SecretLabMagnusPro *arg)
{
	arg->switch_intr();
}

void SecretLabMagnusPro::setup()
{
	this->switch_->setup();

	this->switch_->attach_interrupt(&_switch_intr, this, gpio::INTERRUPT_ANY_EDGE);

	switch_intr();
}

void SecretLabMagnusPro::loop()
{
	recv_controller();
	recv_remote();

	send_controller();

	if (is_remote_on_)
		send_remote();
}

void SecretLabMagnusPro::dump_config()
{
}

void SecretLabMagnusPro::recv_controller()
{
	// throw out old packets
	int available = this->controller_->available();
	if (available > sizeof(this->controller_buf_) * 2)
	{
		uint8_t *buf = new uint8_t[available - sizeof(this->controller_buf_)];
		this->controller_->read_array(buf, available - sizeof(this->controller_buf_));
		delete [] buf;
	}

	while(true)
	{
		// we ran out of bytes, bail out
		if (this->controller_->available() == 0)
			return;

		this->controller_->read_byte(&this->controller_buf_[sizeof(this->controller_buf_) - 1]);

		// is the fist byte the start marker?
		if (this->controller_buf_[0] == 0x5a)
		{
			uint8_t checksum = 0;
			for (int i = 1; i < sizeof(this->controller_buf_) - 1; i++)
				checksum += this->controller_buf_[i];

			// does the checksum match?
			if (checksum == this->controller_buf_[5])
				break;
		}

		// shift out the first byte
		for (int i = 0; i < sizeof(this->controller_buf_) - 1; i++)
			this->controller_buf_[i] = this->controller_buf_[i + 1];
	}

	process_controller();
}

void SecretLabMagnusPro::recv_remote()
{
	if (!is_remote_on_)
		return;

	uint8_t msg[4];

	// Throw out old packets
	if (this->remote_->available() > (sizeof(msg) * 2))
	{
		int read = this->remote_->available() - (sizeof(msg) + 1);
		uint8_t *buf = new uint8_t[read];
		this->remote_->read_array(buf, read);
		delete[] buf;
	}

	while (this->remote_->available() >= sizeof(msg) + 1)
	{
		uint8_t byte;
		this->remote_->read_byte(&byte);
		if (byte == 0xa5)
			break;
	}

	if (this->remote_->available() < sizeof(msg))
		return;

	this->remote_->read_array(msg, sizeof(msg));

	uint8_t checksum = 0;
	for (int i = 0; i < sizeof(msg) - 1; i++)
		checksum += msg[i];

	if (checksum != msg[3])
		return;

	if (msg[1] != (uint8_t)~msg[2])
	{
		ESP_LOGD(TAG, "controller: Keymap not match negated! %02x != %02x", msg[1], (uint8_t)~msg[2]);
		return;
	}

	process_remote(msg[0], msg[1]);
}

void SecretLabMagnusPro::process_controller()
{
	if (this->controller_seg_[0] == this->controller_buf_[1] && this->controller_seg_[1] == this->controller_buf_[2] && this->controller_seg_[2] == this->controller_buf_[3] && this->controller_leds_ == this->controller_buf_[4])
		return;

	this->controller_seg_[0] = this->controller_buf_[1];
	this->controller_seg_[1] = this->controller_buf_[2];
	this->controller_seg_[2] = this->controller_buf_[3];
	this->controller_leds_ = this->controller_buf_[4];

	std::string disp;
	disp += _7seg_to_char(this->controller_seg_[0], true);
	if (this->controller_seg_[0] & 0x80)
		disp += '.';
	disp += _7seg_to_char(this->controller_seg_[1], false);
	if (this->controller_seg_[1] & 0x80)
		disp += '.';
	disp += _7seg_to_char(this->controller_seg_[2], false);
	if (this->controller_seg_[2] & 0x80)
		disp += '.';

	std::string leds_str;
	if (this->controller_leds_ & LED_UP)
		leds_str += "UP ";
	if (this->controller_leds_ & LED_DOWN)
		leds_str += "DOWN ";
	if (this->controller_leds_ & LED_S)
		leds_str += "S ";
	if (this->controller_leds_ & LED_1)
		leds_str += "1 ";
	if (this->controller_leds_ & LED_2)
		leds_str += "2 ";
	if (this->controller_leds_ & LED_3)
		leds_str += "3 ";

	bool is_num = true;
	for (auto chr : disp)
	{
		if ((chr < '0' || chr > '9') && chr != '.')
		{
			is_num = false;
			break;
		}
	}

	if (is_num)
	{
		height_ = std::stof(disp);
		ESP_LOGD(TAG, "height_: %f", height_);
	}

	ESP_LOGD(TAG, "controller: %s %s", disp.c_str(), leds_str.c_str());
}

void SecretLabMagnusPro::send_controller()
{
	uint8_t keys = last_keys_;
	keys &= ~KEY_S;

	if (set_height_ != 0.0)
	{
		ESP_LOGD(TAG, "set_height_: %f height_: %f set_height_ctr_: %d", set_height_, height_, set_height_ctr_);
		keys = 0;
		if (height_ > set_height_ + set_height_fast_limit_)
		{
			ESP_LOGD(TAG, "DOWN");
			keys = KEY_DOWN;
		}
		else if (height_ < set_height_ - set_height_fast_limit_)
		{
			ESP_LOGD(TAG, "UP");
			keys = KEY_UP;
		}
		else
		{
			set_height_ctr_ = (set_height_ctr_ + 1) % set_height_slow_skip_;
			if (height_ > set_height_)
			{
				if (set_height_ctr_ == (set_height_slow_skip_ - 1))
				{
					ESP_LOGD(TAG, "DOWN SLOW");
					keys = KEY_DOWN;
				}
			}
			else if (height_ < set_height_)
			{
				if (set_height_ctr_ == (set_height_slow_skip_ - 1))
				{
					ESP_LOGD(TAG, "UP SLOW");
					keys = KEY_UP;
				}
			}
			else
			{
				ESP_LOGD(TAG, "DONE");
				set_height_ = 0.0;
			}
		}
	}

	uint8_t data[] = {0xa5, last_unk_, keys, (uint8_t) ~keys, (uint8_t)(last_unk_ + keys + ~keys)};
	this->controller_->write_array(data, sizeof(data));
}

void SecretLabMagnusPro::process_remote(uint8_t unk, uint8_t keys)
{
	if (this->last_unk_ == unk && this->last_keys_ == keys)
		return;

	this->last_unk_ = unk;
	this->last_keys_ = keys;

	std::string keys_str;
	if (keys & KEY_S)
		keys_str += "S ";
	if (keys & KEY_1)
		keys_str += "1 ";
	if (keys & KEY_2)
		keys_str += "2 ";
	if (keys & KEY_3)
		keys_str += "3 ";
	if (keys & KEY_UP)
		keys_str += "UP ";
	if (keys & KEY_DOWN)
		keys_str += "DOWN ";

	ESP_LOGD(TAG, "remote: %02x %s", unk, keys_str.c_str());

	if (keys & KEY_S)
	{
		ESP_LOGD(TAG, "remote: set_height_: %f", set_height_);
		if (this->set_height_ == 0.0)
			this->set_height_ = 90.0;
		else
			this->set_height_ = 0.0;
	}
}

void SecretLabMagnusPro::send_remote()
{
	if (!is_remote_on_)
		return;

	uint8_t data[] = {0x5a, this->controller_seg_[0], this->controller_seg_[1], this->controller_seg_[2], this->controller_leds_,
		(uint8_t)(this->controller_seg_[0] + this->controller_seg_[1] + this->controller_seg_[2] + this->controller_leds_)};
	this->remote_->write_array(data, sizeof(data));
}

void SecretLabMagnusPro::switch_intr()
{
	bool state = this->switch_->digital_read();
	ESP_LOGD(TAG, "switch_intr: %d", state);
	is_remote_on_ = !state;
}

} // namespace secretlab
} // namespace esphome