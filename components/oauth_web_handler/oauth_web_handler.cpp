#include "oauth_web_handler.h"
#include "esphome/core/log.h"
#include <esphome/components/logger/logger.h>
#include <regex>
#include <string>

namespace esphome {
namespace oauth_web_handler {

static const char *TAG = "oauth_web_handler";

void OAuthWebHandler::setup() {
    base_->add_handler(this);
    token_pref_ = global_preferences->make_preference<std::string>(this->get_object_id_hash());
    token_pref_.load(token_);
}

void OAuthWebHandler::dump_config() {
    ESP_LOGCONFIG(TAG, "Listening on path /oauth");
}

bool OAuthWebHandler::canHandle(AsyncWebServerRequest *request) {
    ESP_LOGD(TAG, "Checking canHandle %s", request->url().c_str());
    if (std::string(request->url().c_str()).find("/oauth") == 0)
        return true;
    else if (std::string(request->url().c_str()).find("/login") == 0)
        return true;
    return false;
}

void OAuthWebHandler::handleRequest(AsyncWebServerRequest *request) {
    ESP_LOGD(TAG, "Handling oauth");
    std::string url = std::string(request->url().c_str());
    if(request->hasArg("access_token")){
        // Use the token
        request->send(200, "text/javascript", "OK!");
        ESP_LOGD(TAG, "Access token: %s", request->arg("access_token").c_str());
        token_ = std::string(request->arg("access_token").c_str());
        token_pref_.save(token_);
    }
    else if(url=="/oauth") {
        request->send(200, "text/html", "<html><script>window.location.replace(window.location.toString().replace('#','?'));</script></html>");
    }
    else if(url=="/login") {
        request->redirect("https://id.twitch.tv/oauth2/authorize?response_type=token&client_id=<your token here>&redirect_uri=https://teacuppityfanonly.com/oauth&scope=channel%3Amanage%3Apolls+channel%3Aread%3Aredemptions");
    }
    else {
        request->send(200, "text/html", "<html>Error!</html>");
    }
}

bool OAuthWebHandler::isRequestHandlerTrivial() { return false; }

}
}