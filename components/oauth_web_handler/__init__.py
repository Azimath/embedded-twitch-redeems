import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.web_server_base import WebServerBase, CONF_WEB_SERVER_BASE_ID
from esphome.const import CONF_ID

AUTO_LOAD = ['web_server']

oauth_web_handler_ns = cg.esphome_ns.namespace('oauth_web_handler')
OAuthWebHandler = oauth_web_handler_ns.class_('OAuthWebHandler', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(OAuthWebHandler),
    cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(WebServerBase),
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)