#pragma once

#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/core/component.h"
#include "esphome/core/controller.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace oauth_web_handler {

class OAuthWebHandler : public Component, public AsyncWebHandler {
    public:
        OAuthWebHandler(web_server_base::WebServerBase *base) : base_(base) {};
        void setup() override;
        void dump_config() override;

        bool canHandle(AsyncWebServerRequest *request) override;
        void handleRequest(AsyncWebServerRequest *request) override;
        bool isRequestHandlerTrivial() override;
        std::string getToken() { return token_; };
        std::string getAuthorization() { return "Bearer " << token_; }
        bool hasToken() { return token_.length() > 0; }
    protected:
        web_server_base::WebServerBase *base_;
        std::string state_;
        std::string token_;
        ESPPreferenceObject token_pref_;
};

} // namespace oauth_web_handler
} // namespace esphome