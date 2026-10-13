
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
#include <sstream>
#include <optional>
#include <chrono>


class Component {
public:
    virtual ~Component() = default;

    virtual void enqueue(const Message &message) = 0;
};


enum ComponentRole {
    Consumer = 0x01,
    Producer = 0x02
};

class AbstractComponent : public Component {
public:
    AbstractComponent(MessageBus *messageBus, const ComponentRole &roleFlags) : messageBus(messageBus), roleFlags(roleFlags) {
        assert(roleFlags);
    }

    void start() {
        setRunnning(true);

        if (roleFlags & ComponentRole::Producer) {
            producer = std::thread(&AbstractComponent::producerLoop, this);
        }

        if (roleFlags & ComponentRole::Consumer) {
            consumer = std::thread(&AbstractComponent::consumerLoop, this);
        }
    }

    void shutdown() {
        setRunnning(false);

        if (roleFlags & ComponentRole::Producer) {
            producer.join();
        }

        if (roleFlags & ComponentRole::Consumer) {
            consumer.join();
        }
    }

    void enqueue(const Message &message) override {
        std::lock_guard<std::mutex> guard(messagesMutex);
        messages.push_back(message);
    }

    bool hasMessages() {
        return !messages.empty();
    }

    Message consume() {
        std::lock_guard<std::mutex> guard(messagesMutex);
        Message message = messages.front();
        messages.erase(messages.begin());

        return message;
    }

    bool isRunning() const {
        return running;
    }

    MessageBus* getMessageBus() {
        return messageBus;
    }

    void setRunnning(bool value) {
        std::lock_guard<std::mutex> guard(runningMutex);
        running = value;
    }

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
