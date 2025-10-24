from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

CODEOWNERS = ["@balika011"]
DEPENDENCIES = ["uart"]

CONF_CONTROLLER = "controller"
CONF_CONTROLLER_KEY = "controller_key"
CONF_REMOTE = "remote"
CONF_REMOTE_KEY = "remote_key"
CONF_SWITCH = "switch"

secretlab_ns = cg.esphome_ns.namespace("secretlab")
SecretLabMagnusPro = secretlab_ns.class_("SecretLabMagnusPro", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SecretLabMagnusPro),
        cv.GenerateID(CONF_CONTROLLER): cv.use_id(uart.UARTComponent),
        cv.Required(CONF_CONTROLLER_KEY): pins.gpio_output_pin_schema,
        cv.GenerateID(CONF_REMOTE): cv.use_id(uart.UARTComponent),
        cv.Required(CONF_REMOTE_KEY): pins.gpio_input_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    controller = await cg.get_variable(config[CONF_CONTROLLER])
    cg.add(var.set_controller(controller))

    controller_key = await cg.gpio_pin_expression(config[CONF_CONTROLLER_KEY])
    cg.add(var.set_controller_key(controller_key))

    remote = await cg.get_variable(config[CONF_REMOTE])
    cg.add(var.set_remote(remote))

    remote_key = await cg.gpio_pin_expression(config[CONF_REMOTE_KEY])
    cg.add(var.set_remote_key(remote_key))

    switch = await cg.gpio_pin_expression(config[CONF_SWITCH])
    cg.add(var.set_switch(switch))

    await cg.register_component(var, config)
