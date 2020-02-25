#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace keypad_sensor {

// Thresholds in microseconds
struct KeypadSensorThresholds {
    uint32_t min_pulse_width{0}, wide_pulse_threshold{600}, start_bit_min_width{1500};
};

// ISR-safe (without vtables in flash) storage for the KeypadSensor.
class KeypadSensorStore {
public:
    KeypadSensorStore(const KeypadSensorThresholds &thresh);
    void setup(GPIOPin *pin);
    volatile uint8_t publish{0};

protected:
    static void gpio_intr(KeypadSensorStore *s);
    void gpio_intr();

    bool was_on_{0};
    uint8_t curr_value_{0};
    int8_t curr_bit_{-1}; // -1 == start bit
    uint32_t last_rising_micros_{0};

    ISRInternalGPIOPin *isr_pin_;
    const KeypadSensorThresholds &thresh_;
};


class KeypadSensor : public sensor::Sensor, public Component {
public:
    void set_pin(GPIOPin *pin);
    void setup() override;
    void loop() override;

protected:
    GPIOPin *pin_;
    KeypadSensorThresholds thresh_;
    KeypadSensorStore store_{thresh_};
};
} // namespace keypad_sensor
} // namespace esphome
