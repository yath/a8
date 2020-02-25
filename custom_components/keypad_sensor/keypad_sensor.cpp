#include "keypad_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace keypad_sensor {

static const char *TAG = "keypad_sensor";

// static callback
void ICACHE_RAM_ATTR KeypadSensorStore::gpio_intr(KeypadSensorStore *s) {
    s->gpio_intr();
}

void ICACHE_RAM_ATTR KeypadSensorStore::gpio_intr() {
    const bool is_on = this->isr_pin_->digital_read();
    if (this->was_on_ == is_on)
        return;
    this->was_on_ = is_on;

    const uint32_t now = micros();

    if (is_on) { /* rising */
        this->last_rising_micros_ = now;
    } else { /* falling */
        const uint32_t on_time = now - this->last_rising_micros_;
        this->last_rising_micros_ = 0;

        if (this->curr_bit_ < 0) {
            /* start bit */
            if (on_time < this->thresh_.start_bit_min_width) {
                return; // too short, ignore
            }
        } else {
            bool is_wide = on_time > this->thresh_.wide_pulse_threshold;
            this->curr_value_ = (this->curr_value_ << 1) | is_wide;
        }
        this->curr_bit_++;

        if (this->curr_bit_ == 8) {
            this->publish = this->curr_value_;
            this->curr_value_ = 0;
            this->curr_bit_ = -1;
        }
    }
}

KeypadSensorStore::KeypadSensorStore(const KeypadSensorThresholds &thresh) :
    thresh_(thresh)
{ }

void KeypadSensorStore::setup(GPIOPin *pin) {
    pin->setup();
    this->isr_pin_ = pin->to_isr();
    pin->attach_interrupt(KeypadSensorStore::gpio_intr, this, CHANGE);
}

void KeypadSensor::set_pin(GPIOPin *pin)  {
    this->pin_ = pin;
}

void KeypadSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Keypad sensor '%s'.", this->name_.c_str());
    this->store_.setup(this->pin_);
}

void KeypadSensor::loop() {
    if (uint8_t val = this->store_.publish) {
        this->store_.publish = 0;
        ESP_LOGI(TAG, "Publishing value 0x%02x", val);
        publish_state(val);
    }
}


} // namespace keypad_sensor
} // namespace esphome
