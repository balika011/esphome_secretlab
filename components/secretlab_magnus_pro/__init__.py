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
        cv.GenerateID(CONF_CONTROLLER): cv.use_id(uart.UARTComponent),
        cv.GenerateID(CONF_REMOTE): cv.use_id(uart.UARTComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    controller = await cg.get_variable(config[CONF_CONTROLLER])
    remote = await cg.get_variable(config[CONF_REMOTE])
    #cg.add(var.set_controller(controller))
    #cg.add(var.set_remote(remote))
    await cg.register_component(var, config)
