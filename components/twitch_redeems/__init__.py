import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_TIMEOUT,
    CONF_TRIGGER_ID
)
from esphome.core import CORE
from esphome.components import oauth_web_handler

CONF_ON_REDEEM_ID = "on_redeem_id"
CONF_REDEEM_ID = "redeem_id"
CONF_ON_REDEEM_CONTAINS = "on_redeem_contains"
CONF_REDEEM_CONTAINS = "redeem_contains"
CONF_OAUTH_COMPONENT = "oauth_component"

DEPENDENCIES = ["network"]

twitch_redeems_ns = cg.esphome_ns.namespace("twitch_redeems")
TwitchRedeemClient = twitch_redeems_ns.class_("TwitchRedeemClient", cg.Component)
TwitchRedeemIDTrigger = twitch_redeems_ns.class_("TwitchRedeemIDTrigger", automation.Trigger.template())
TwitchRedeemContainsTrigger = twitch_redeems_ns.class_("TwitchRedeemContainsTrigger", automation.Trigger.template())

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(TwitchRedeemClient),
    cv.Optional(CONF_ON_REDEEM_ID): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TwitchRedeemIDTrigger),
            cv.Required(CONF_REDEEM_ID): cv.string
        }
    ),
    cv.Optional(CONF_ON_REDEEM_CONTAINS): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TwitchRedeemContainsTrigger),
            cv.Required(CONF_REDEEM_CONTAINS): cv.string
        }
    ),
    cv.GenerateID(CONF_OAUTH_COMPONENT): cv.use_id(oauth_web_handler.OAuthWebHandler),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_OAUTH_COMPONENT])

    if CORE.is_esp32:
        cg.add_library("WiFiClientSecure", None)
        cg.add_library("HTTPClient", None)
    if CORE.is_esp8266:
        cg.add_define("USE_HTTP_REQUEST_ESP8266_HTTPS")
        cg.add_library("ESP8266HTTPClient", None)
        
    cg.add_library("ArduinoWebSockets=https://github.com/hesa2020/ArduinoWebsockets", None)

    await cg.register_component(var, config)

    for conf in config.get(CONF_ON_REDEEM_ID):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID], var, conf[CONF_REDEEM_ID]
        )
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_REDEEM_CONTAINS):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID], var, conf[CONF_REDEEM_CONTAINS]
        )
        await automation.build_automation(trigger, [], conf)
