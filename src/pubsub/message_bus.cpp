#include "pubsub/message_bus.h"

namespace pubsub {

void MessageBus::subscribe(EventType type, EventCallback callback) {
    subscribers[type].push_back(callback);
}

void MessageBus::publish(const Event& event) {
    auto it = subscribers.find(event.type);
    if (it != subscribers.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

}
