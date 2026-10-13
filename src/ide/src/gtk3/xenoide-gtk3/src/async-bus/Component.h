
#pragma once

#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include <tuple>

enum ComponentRole {
    Consumer = 0x01,
    Producer = 0x02
};

struct Message;

class MessageBus;
class Component {
public:
    Component(MessageBus *messageBus, const ComponentRole &roleFlags);

    virtual ~Component();

    void start();

    void shutdown();

    void enqueue(const Message &message);

    bool hasMessages();

    Message consume();

    bool isRunning() const;

    MessageBus* getMessageBus();

    void setRunnning(bool value);

protected:
    virtual void producerLoop() {}

    virtual void consumerLoop() {}

private:
    MessageBus *messageBus = nullptr;
    ComponentRole roleFlags;

    std::thread consumer;
    std::thread producer;

    bool running = false;
    std::mutex runningMutex;

    //! Enqueued messages
    std::vector<Message> messages;
    std::mutex messagesMutex;
};
