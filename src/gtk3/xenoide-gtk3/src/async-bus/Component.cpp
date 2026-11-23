
#include "Component.h"
#include "MessageBus.h"

#include <cassert>

Component::Component(MessageBus *messageBus, const ComponentRole &roleFlags) : messageBus(messageBus), roleFlags(roleFlags) {
        assert(roleFlags);
    }

Component::~Component() {}

void Component::start() {
    setRunnning(true);

    if (roleFlags & ComponentRole::Producer) {
        producer = std::thread(&Component::producerLoop, this);
    }

    if (roleFlags & ComponentRole::Consumer) {
        consumer = std::thread(&Component::consumerLoop, this);
    }
}

void Component::shutdown() {
    setRunnning(false);

    if (roleFlags & ComponentRole::Producer) {
        producer.join();
    }

    if (roleFlags & ComponentRole::Consumer) {
        consumer.join();
    }
}

void Component::enqueue(const Message &message) {
    std::lock_guard<std::mutex> guard(messagesMutex);
    messages.push_back(message);
}

bool Component::hasMessages() {
    return !messages.empty();
}

Message Component::consume() {
    std::lock_guard<std::mutex> guard(messagesMutex);
    Message message = messages.front();
    messages.erase(messages.begin());

    return message;
}

bool Component::isRunning() const {
    return running;
}

MessageBus* Component::getMessageBus() {
    return messageBus;
}

void Component::setRunnning(bool value) {
    std::lock_guard<std::mutex> guard(runningMutex);
    running = value;
}
