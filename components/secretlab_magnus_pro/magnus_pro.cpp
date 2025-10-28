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

static char _7seg_to_char(uint8_t seg)
{
	static const uint8_t alpha_7seg[] = {0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x3d, 0x74, 0x30, 0x1e, 0x75, 0x38, 0x15, 0x37, 0x3f, 0x73, 0x67, 0x33, 0x6d, 0x78, 0x3e, 0x2e, 0x2a, 0x76, 0x6e, 0x4b};
	static const uint8_t num_7seg[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

	seg &= 0x7f;

	if (seg == 0x00) return ' ';

	if (seg == 0x40)
		return '-';

	for (int i = 0; i < sizeof(alpha_7seg); i++)
		if (alpha_7seg[i] == seg)
			return 'A' + i;

	for (int i = 0; i < sizeof(num_7seg); i++)
		if (num_7seg[i] == seg)
			return '0' + i;

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

void SecretLabMagnusPro::process_controller(uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t leds)
{
	if (this->last_seg_[0] == seg1 && this->last_seg_[1] == seg2 && this->last_seg_[2] == seg3 && this->last_leds_ == leds)
		return;

	this->last_seg_[0] = seg1;
	this->last_seg_[1] = seg2;
	this->last_seg_[2] = seg3;
	this->last_leds_ = leds;

	std::string disp;
	disp += _7seg_to_char(seg1);
	if (seg1 & 0x80)
		disp += '.';
	disp += _7seg_to_char(seg2);
	if (seg2 & 0x80)
		disp += '.';
	disp += _7seg_to_char(seg3);
	if (seg3 & 0x80)
		disp += '.';

	std::string leds_str;
	if (leds & LED_UP)
		leds_str += "UP ";
	if (leds & LED_DOWN)
		leds_str += "DOWN ";
	if (leds & LED_S)
		leds_str += "S ";
	if (leds & LED_1)
		leds_str += "1 ";
	if (leds & LED_2)
		leds_str += "2 ";
	if (leds & LED_3)
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
		height_ = std::stof(disp);

	ESP_LOGD(TAG, "controller: %s %s", disp.c_str(), leds_str.c_str());
}

void SecretLabMagnusPro::send_controller()
{
	uint8_t keys = last_keys_;
	keys &= ~KEY_S;

	if (do_shit_)
	{
		if (height_ > 92)
			keys = KEY_DOWN;
		else if (height_ < 88)
			keys = KEY_UP;
		else
			do_shit_ = false;
	}

	uint8_t data[] = {0xa5, last_unk_, keys, (uint8_t) ~keys, (uint8_t)(last_unk_ + keys + ~keys)};
	this->controller_->write_array(data, sizeof(data));
}

void SecretLabMagnusPro::recv_remote()
{
	if (!is_remote_on_)
	{
		while (this->remote_->available() > 0)
		{
			uint8_t byte;
			this->remote_->read_byte(&byte);
		}
		return;
	}

	uint8_t msg[4];

	while (this->remote_->available() >= sizeof(msg) + 1)
	{
		uint8_t byte;
		this->remote_->read_byte(&byte);
		if (byte == 0xa5)
			break;

		ESP_LOGD(TAG, "remote: %02x != a5", byte);
	}

	if (this->remote_->available() < sizeof(msg))
		return;

	this->remote_->read_array(msg, sizeof(msg));

	uint8_t checksum = 0;
	for (int i = 0; i < sizeof(msg) - 1; i++)
		checksum += msg[i];

	if (checksum != msg[3])
	{
		ESP_LOGD(TAG, "remote: Invalid checksum! %02x != %02x", checksum, msg[3]);
		ESP_LOGD(TAG, "remote: msg: %02x %02x %02x %02x", msg[0], msg[1], msg[2], msg[3]);
		return;
	}

	if (msg[1] != (uint8_t)~msg[2])
	{
		ESP_LOGD(TAG, "controller: Keymap not match negated! %02x != %02x", msg[1], (uint8_t)~msg[2]);
		return;
	}

	process_remote(msg[0], msg[1]);
}

void SecretLabMagnusPro::process_remote(uint8_t unk, uint8_t keys)
{
	if (this->last_unk_ == unk && this->last_keys_ == keys)
		return;

	this->last_unk_ = unk;
	this->last_keys_ = keys;

	ESP_LOGD(TAG, "remote: %02x %02x", unk, keys);

	if (keys & KEY_S)
	{
		this->do_shit_ = true;
	}
}

void SecretLabMagnusPro::send_remote()
{
	if (!is_remote_on_)
		return;

#if 0
	uint8_t data[] = {0x5a, last_seg_[0], last_seg_[1], last_seg_[2], last_leds_, (uint8_t)(last_seg_[0] + last_seg_[1] + last_seg_[2] + last_leds_)};
	this->remote_->write_array(data, sizeof(data));
#else
	//                                     A     B     C     D     E     F    G      H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W     X     Y     Z
	static const uint8_t alpha_7seg[] = {0x77, 0x7f, 0x39, 0x5e, 0x79, 0x71, 0x3d, 0x74, 0x30, 0x1e, 0x75, 0x38, 0x15, 0x37, 0x3f, 0x73, 0x67, 0x33, 0x6d, 0x78, 0x3e, 0x2e, 0x2a, 0x76, 0x6e, 0x4b};
	uint8_t seg0 = alpha_7seg[3];
	uint8_t seg1 = alpha_7seg[4];
	uint8_t seg2 = alpha_7seg[5];
	uint8_t data[] = {0x5a, seg0, seg1, seg2, last_leds_, (uint8_t)(seg0 + seg1 + seg2 + last_leds_)};
	this->remote_->write_array(data, sizeof(data));
#endif
}

void SecretLabMagnusPro::switch_intr()
{
	bool state = this->switch_->digital_read();
	ESP_LOGD(TAG, "switch_intr: %d", state);
	is_remote_on_ = !state;
}

} // namespace secretlab
} // namespace esphome