#pragma once 

#include <iosfwd>
#include <string>
#include <thread>
#include <vector>
#include <tuple>
#include <set>

enum class MessageKind {
    Event,
    Command,
    Query
};

std::ostream& operator<< (std::ostream &os, MessageKind kind);

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

class Component;
class MessageBus {
public:
    void registerComponent(Component *component, const MessageTypeSet &supportedMessageTypes = {});

    void unregisterComponent(Component *component);

    void publishMessage(const Message &message);

private:
    std::vector<std::pair<Component*, MessageTypeSet>> components;
};
