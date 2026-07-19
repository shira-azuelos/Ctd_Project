#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <any>

namespace pubsub {

enum class EventType {
    SCORE_CHANGED,
    MOVE_LOGGED,
    PLAY_SOUND,
    GAME_STATUS
};

struct Event {
    EventType type;
    std::any payload;
};

struct ScorePayload {
    std::string color;
    int new_score;
};

struct SoundPayload {
    std::string sound_name;
};

using EventCallback = std::function<void(const Event&)>;

class MessageBus {
private:
    std::map<EventType, std::vector<EventCallback>> subscribers;
    MessageBus() = default;

public:
    static MessageBus& get_instance() {
        static MessageBus instance;
        return instance;
    }

    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    void subscribe(EventType type, EventCallback callback);
    void publish(const Event& event);
};

}
