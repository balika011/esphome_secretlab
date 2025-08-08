from esphome import automation
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

CODEOWNERS = ["@balika011"]
DEPENDENCIES = ["uart"]

CONF_CONTROLLER = "controller"
CONF_REMOTE = "remote"

secretlab_ns = cg.esphome_ns.namespace("secretlab")
SecretLabMagnusPro = secretlab_ns.class_("SecretLabMagnusPro", cg.Component)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SecretLabMagnusPro),
        cv.Optional(CONF_CONTROLLER): cv.Schema(
            {
                cv.GenerateID(): cv.declare_id(uart.UARTDevice),
            }
        ).extend(uart.UART_DEVICE_SCHEMA),
        cv.Optional(CONF_REMOTE): cv.Schema(
            {
                cv.GenerateID(): cv.declare_id(uart.UARTDevice),
            }
        ).extend(uart.UART_DEVICE_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    controller_config = config.get(CONF_CONTROLLER)
    controller_var = cg.new_Pvariable(controller_config[CONF_ID])
    await uart.register_uart_device(controller_var, controller_config)

    remote_config = config.get(CONF_REMOTE)
    remote_var = cg.new_Pvariable(remote_config[CONF_ID])
    await uart.register_uart_device(remote_var, remote_config)

    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_controller(paren))
    cg.add(var.set_remote(paren))
    await cg.register_component(var, config)
