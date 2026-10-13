#pragma once 

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <atomic>
#include <vector>
#include <tuple>
#include <algorithm>
#include <cassert>

enum class MessageKind {
    Event,
    Command,
    Query
};

inline std::ostream& operator<< (std::ostream &os, MessageKind kind) {
    switch (kind) {
        case MessageKind::Event: os << "Event"; break;
        case MessageKind::Command: os << "Command"; break;
        case MessageKind::Query: os << "Query"; break;
        default: os << "<UnknownMessageKind>"; break;
    }

    return os;
}

using MessageType = uint32_t;

//! Message structure, created by Producers and processed by Consumers
//! Delivered by the MessageBus
//! TODO: Add Payload
struct Message {
    //! It is a Event, Command or Query?
    MessageKind kind = MessageKind::Event;

    //! Generated unique identifier
    uint32_t uuid = 0;          

    //! When it was created
    uint64_t timestamp = 0;     

    //! The specific type of Event. Created dynamically
    MessageType type = 0;          

    //! Who should carry on this Command or Query
    uint32_t consumerId = 0;    
};


class MessageBus;

using MessageTypeSet = std::set<MessageType>;

class Component {
public:
    virtual ~Component() = default;

    virtual void enqueue(const Message &message) = 0;
};

class MessageBus {
public:
    void registerComponent(Component *component, const MessageTypeSet &supportedMessageTypes = {});

    void unregisterComponent(Component *component);

    void publish(const Message &message);

private:
    std::vector<std::pair<Component*, MessageTypeSet>> components;
};
