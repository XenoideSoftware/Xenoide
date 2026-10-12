
#include "message-bus.h"

#include <sstream>

std::mutex stdoutMutex;
void logger(std::thread::id threadId, std::string msg) {
    std::lock_guard<std::mutex> guard(stdoutMutex);
    std::cout << "[" << threadId << "] " << msg << std::endl;
}

class LoggerComponent : Component {
public:
    LoggerComponent(MessageBus *messageBus) : messageBus(messageBus) {}

    void start() {
        std::cout << "Initializing LoggerComponent" << std::endl;

        running = true;

        thread = std::thread([this]() {
            while (running) {
                // TODO: Replace with a condition variable.
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (hasMessages()) {
                    logMessage(consume());
                }
            }
        });
    }

    void shutdown() {
        std::cout << "Shutting down LoggerComponent" << std::endl;
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

    void logMessage(const Message &message) {
        std::thread::id threadId = std::this_thread::get_id();
        
        std::stringstream ss;

        ss  << "Message { "
            << ".kind = " << message.kind << ", "
            << ".uuid = " << message.uuid << ", "
            << ".timestamp = " << message.timestamp << ", "
            << ".type = " << message.type << ", "
            << ".consumerId = " << message.consumerId 
            << " }";

        logger(threadId, ss.str());
    }

private:
    MessageBus *messageBus = nullptr;

    //! always runs unless is stopped via the Shutdown method
    std::thread thread;

    bool running = false;
    std::mutex runningMutex;

    //! Enqueued messages
    std::vector<Message> messages;

    std::mutex messagesMutex;
};

int main() {
    MessageBus messageBus;

    return 0;
}
