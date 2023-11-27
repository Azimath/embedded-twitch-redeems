#include "twitch_redeems.h"
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"

using namespace websockets;

namespace esphome {
namespace twitch_redeems {

static const char *TAG = "twitch_redeems";
WebsocketsClient webSocket;
bool connected;
bool welcomed;
bool validated;
bool subscribed;
std::string session_id;
CallbackManager<void(std::string, std::string)> redeem_callbacks{};

void eventCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        ESP_LOGD(TAG,"Connnection Opened");
        connected = true;
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        ESP_LOGD(TAG,"Connnection Closed");
        connected = false;
        welcomed = false;
        validated = false;
        subscribed = false;
    } else if(event == WebsocketsEvent::GotPing) {
        ESP_LOGD(TAG,"Got a Ping!");
        webSocket.pong();
    } else if(event == WebsocketsEvent::GotPong) {
        ESP_LOGD(TAG,"Got a Pong!");
    }
}

void messageCallback(WebsocketsMessage message) {
    ESP_LOGD(TAG, "Got Message: ");
    ESP_LOGD(TAG, "%s", message.rawData().c_str());
    DynamicJsonDocument root(4096);
    if(deserializeJson(root, message.rawData()) == DeserializationError::Ok) {
        std::string message_type = root["metadata"]["message_type"].as<std::string>();
        ESP_LOGD(TAG, "%s", message_type.c_str());
        if(message_type == "session_welcome") {
            session_id = root["payload"]["session"]["id"].as<std::string>();
            ESP_LOGD(TAG, "Session ID: %s", session_id.c_str());
            welcomed = true;
        }
        else if (message_type == "notification") {
            std::string subscription_type = root["payload"]["subscription"]["type"].as<std::string>();
            if (subscription_type == "channel.channel_points_custom_reward_redemption.add") {
                std::string redeemId = root["payload"]["event"]["reward"]["id"].as<std::string>();
                std::string redeemTitle = root["payload"]["event"]["reward"]["title"].as<std::string>();
                ESP_LOGD(TAG, "Got redemption %s : %s", redeemId.c_str(), redeemTitle.c_str());
                redeem_callbacks.call(redeemId, redeemTitle);
            }
        }
    }
    else {
        ESP_LOGW(TAG, "Deserialization error!");
    }
}

void TwitchRedeemClient::setup() {
    connected = false;
    welcomed = false;
    validated = false;
    subscribed = false;
    webSocket.setInsecure();
    webSocket.onEvent(eventCallback);
    webSocket.onMessage(messageCallback);
    ESP_LOGD(TAG, "Redeems started");
};

void TwitchRedeemClient::dump_config() {
    ESP_LOGCONFIG(TAG, "Redeems");
};

void TwitchRedeemClient::loop() {
    if (welcomed && !validated && !subscribed && oauth_.hasToken()) {
        this->client_.begin("http://192.168.1.34:8080/oauth/validate");
        client_.addHeader("Authorization", oauth_.getAuthorization());
        int httpCode = client_.sendRequest("GET");
        if(httpCode == 200){
            String payload = client_.getString();
            DynamicJsonDocument root(2048);
            if(deserializeJson(root, message.rawData()) == DeserializationError::Ok) {
                user_id_ = root["user_id"].as<std::string>();
            }
            validated = true;
        }
        else {
            ESP_LOGE(TAG, "Validation failed: return code %d", httpCode);
            this->mark_failed();
        }
        this->client_.end();
    }
    if (welcomed && validated && !subscribed) {
        this->client_.begin("http://192.168.1.34:8080/eventsub/subscriptions");
        DynamicJsonDocument root(2048);
        root["type"] = "channel.channel_points_custom_reward_redemption.add";
        root["version"] = "1";
        root["condition"]["broadcaster_user_id"] = user_id_;
        root["transport"]["method"] = "websocket";
        root["transport"]["session_id"] = session_id;
        std::string jsonToSend;
        serializeJson(root, jsonToSend);
        ESP_LOGD(TAG, "%s", jsonToSend.c_str());
        client_.addHeader("Authorization", oauth_.getAuthorization());
        client_.addHeader("Client-Id", "5qz9ov1uh7jl3kpimuuf0frwqs9ag3");
        client_.addHeader("Content-Type", "application/json");
        client_.sendRequest("POST", jsonToSend.c_str());
        this->client_.end();
        subscribed = true;
    }
    if(connected) {
        webSocket.poll();
    } else if (network::is_connected()) {
        ESP_LOGD(TAG, "Connecting to wss");
        //webSocket.connect("wss://eventsub.wss.twitch.tv/ws");
        webSocket.connect("ws://192.168.1.34:8080/ws");
    }
};

void TwitchRedeemClient::add_on_redeem_callback(std::function<void(std::string, std::string)> &&callback) {
    redeem_callbacks.add(std::move(callback));
}

} // namespace twitch_redeems
} // namespace esphome