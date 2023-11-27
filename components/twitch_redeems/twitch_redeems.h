#pragma once
#include "esphome/core/component.h"
#include "esphome/components/json/json_util.h"
#include "esphome/core/automation.h"
#include <algorithm>
#include "components/oauth_web_handler/oauth_web_handler.h"

#ifdef USE_ESP32
    #include <HTTPClient.h>
#endif
#ifdef USE_ESP8266
    #include <ESP8266HTTPClient.h>
    #ifdef USE_HTTP_REQUEST_ESP8266_HTTPS
        #include <WiFiClientSecure.h>
    #endif
#endif

#include <ArduinoWebsockets.h>

namespace esphome {
namespace twitch_redeems {

class TwitchRedeemClient : public Component {
    public:
        TwitchRedeemClient(oauth_web_handler::OAuthWebHandler *oauth) : oauth_(oauth) {};
        void setup() override;
        void dump_config() override;
        void loop() override;
        void add_on_redeem_callback(std::function<void(std::string, std::string)> &&callback);
    protected:
        HTTPClient client_{};
        oauth_web_handler::OAuthWebHandler *oauth_;
};

class TwitchRedeemIDTrigger : public Trigger<> {
    public:
        TwitchRedeemIDTrigger(TwitchRedeemClient* parent, std::string redeem_id) 
            : redeem_id_(redeem_id) {
            parent->add_on_redeem_callback([this](std::string redeem_id, std::string redeem_title) {
                if(redeem_id == this->redeem_id_){
                    this->trigger();
                }
            });
        }
    protected:
        std::string redeem_id_;
};

class TwitchRedeemContainsTrigger : public Trigger<> {
    public:
        TwitchRedeemContainsTrigger(TwitchRedeemClient* parent, std::string redeem_contains) {
            std::transform(redeem_contains.begin(), redeem_contains.end(), redeem_contains.begin(), [&](char c){return std::tolower(c);});
            redeem_contains_ = redeem_contains;
            parent->add_on_redeem_callback([this](std::string redeem_id, std::string redeem_title) {
                std::transform(redeem_title.begin(), redeem_title.end(), redeem_title.begin(), [&](char c){return std::tolower(c);});
                if(redeem_title.find(this->redeem_contains_) != std::string::npos){
                    this->trigger();
                }
            });
        }
    protected:
        std::string redeem_contains_;
        std::string user_id_;
};



} // namespace twitch_redeems
} // namespace esphome