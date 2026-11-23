
#include "MessageBus.h"
#include "Component.h"

#include <ostream>
#include <cassert>
#include <algorithm>

std::ostream& operator<< (std::ostream &os, MessageKind kind) {
    switch (kind) {
        case MessageKind::Event: os << "Event"; break;
        case MessageKind::Command: os << "Command"; break;
        case MessageKind::Query: os << "Query"; break;
        default: os << "<UnknownMessageKind>"; break;
    }

    return os;
}


void MessageBus::registerComponent(Component *component, const MessageTypeSet &supportedMessageTypes) {
    assert(component);

    auto pair = std::make_pair(component, supportedMessageTypes);
    
    components.push_back(pair);
}

void MessageBus::unregisterComponent(Component *component) {
    auto it = std::find_if(components.begin(), components.end(), [component](const auto &pair) {
        return component == pair.first;
    });

    assert(it != components.end());
    
    components.erase(it);
}

void MessageBus::publishMessage(const Message &message) {
    for (auto &pair : components) {
        Component *component = pair.first;
        std::set<MessageType> messageTypes = pair.second;

        if (messageTypes.empty() || messageTypes.count(message.type) > 0) {
            component->enqueue(message);
        }
    }
}
