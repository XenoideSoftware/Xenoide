
#include "MessageBus.h"
#include "Component.h"

std::mutex stdoutMutex;
void logline(std::thread::id threadId, std::string msg) {
    std::lock_guard<std::mutex> guard(stdoutMutex);
    std::cout << "[" << threadId << "] " << msg << std::endl;
}

void logline(std::string msg) {
    std::lock_guard<std::mutex> guard(stdoutMutex);
    std::cout << msg << std::endl;
}

namespace {
    std::atomic<uint32_t> currentUuid = 0;
}

Message createMessage(MessageKind kind, MessageType type) {
    using std::chrono::duration_cast;
    using std::chrono::system_clock;
    using std::chrono::milliseconds;

    Message msg;

    msg.kind = kind;
    msg.type = type;
    msg.uuid = ++currentUuid;
    msg.timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    return msg;
}

Message createEvent(MessageType type) {
    return createMessage(MessageKind::Event, type);
}

Message createCommand(MessageType type) {
    return createMessage(MessageKind::Command, type);
}

Message createQuery(MessageType type) {
    return createMessage(MessageKind::Query, type);
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

    logline(threadId, ss.str());
}

enum MessageType2 {
    TICK = 1
};

class LoggerComponent : public Component {
public:
    explicit LoggerComponent(MessageBus *messageBus) 
        : Component(messageBus, ComponentRole::Consumer) {}

    void consumerLoop() override {
        while (isRunning()) {
            // TODO: Replace with a condition variable.
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (hasMessages()) {
                logMessage(consume());
            }
        }
    }
};

struct AppState {
    bool running = true;
    int maxTickCount = 5;
    int currentTickCount = 0;
};

std::string to_string(const AppState &appState) {
    std::stringstream ss;

    ss << "AppState {";
    ss << ".running: " << appState.running << ", ";
    ss << ".maxTickCount: " << appState.maxTickCount << ", ";
    ss << ".currentTickCount: " << appState.currentTickCount << "}";
    
    return ss.str();
}

class AppControlComponent : public Component {
public:
    explicit AppControlComponent(MessageBus *messageBus, AppState *appState) 
        : Component(messageBus, ComponentRole::Consumer), appState(appState) {

        assert(appState);
    }

    void consumerLoop() override {
        while (isRunning()) {
            // TODO: Replace with a condition variable.
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (!hasMessages()) {
                continue;
            }

            Message message = consume();

            if (message.type == MessageType2::TICK) {
                if (++(appState->currentTickCount) >= appState->maxTickCount) {
                    appState->running = false;
                }
            }
        }
    }

private:
    AppState *appState = nullptr;
};

class TimerComponent : public Component {
public:
    TimerComponent(MessageBus *messageBus) : Component(messageBus, ComponentRole::Producer) {}

    void producerLoop() override {
        while (isRunning()) {
            Message event = createEvent(MessageType2::TICK);
            getMessageBus()->publishMessage(event);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

int main() {
    AppState appState;
    appState.running = true;
    appState.maxTickCount = 3;
    appState.currentTickCount = 0;

    MessageBus messageBus;
    LoggerComponent logger(&messageBus);
    TimerComponent timer(&messageBus);
    AppControlComponent appControl(&messageBus, &appState);

    messageBus.registerComponent(&logger);
    messageBus.registerComponent(&timer);
    messageBus.registerComponent(&appControl);

    logger.start();
    timer.start();
    appControl.start();

    while (appState.running) {
        logline(to_string(appState));

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    logline(to_string(appState));

    logger.shutdown();
    timer.shutdown();
    appControl.shutdown();

    messageBus.unregisterComponent(&logger);
    messageBus.unregisterComponent(&timer);
    messageBus.unregisterComponent(&appControl);

    return 0;
}
