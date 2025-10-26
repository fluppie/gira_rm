import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@fluppie"]
DEPENDENCIES = ["uart"]

CONF_UART_ID = "uart_id"

gira_rm_ns = cg.esphome_ns.namespace("gira_rm")
GiraRMComponent = gira_rm_ns.class_("GiraRMComponent", cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GiraRMComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    uart_comp = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_comp))
    await cg.register_component(var, config)
