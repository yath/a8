import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_PIN

DEPENDENCIES = ['sensor']

keypad_ns = cg.esphome_ns.namespace('keypad_sensor')
KeypadSensor = keypad_ns.class_('KeypadSensor', sensor.Sensor, cg.Component)

CONFIG_SCHEMA = sensor.sensor_schema("", "", 0).extend({
    cv.GenerateID(): cv.declare_id(KeypadSensor),
    cv.Required(CONF_PIN): cv.All(pins.internal_gpio_input_pin_schema,
                                  pins.validate_has_interrupt),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)

    pin = yield cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
