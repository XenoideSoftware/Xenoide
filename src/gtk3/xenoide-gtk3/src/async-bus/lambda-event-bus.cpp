// Lambda-based, multithreaded, event bus


#include <iostream>
#include <string>
#include <map>
#include <set>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <atomic>

std::mutex stdoutMutex;

void logger(std::thread::id threadId, std::string msg) {
    std::lock_guard<std::mutex> guard(stdoutMutex);
    std::cout << "[" << threadId << "] " << msg << std::endl;
}

typedef std::map<std::string, std::string> EventPayload; 

struct Event {
    std::string type;
    EventPayload payload;

    Event(std::string type) : type(type) {}
    Event(std::string type, EventPayload payload) : type(type), payload(payload) {}
};

typedef std::function<void (Event evt)> EventFn;

const std::string EVENT_APP_TERMINATE = "app-terminate";
const std::string EVENT_COUNT_INCREASE = "count-increase";
const std::string EVENT_COUNT_INCREASED = "count-increased";

class EventBus {
public:
    void registerProcessor(EventFn eventFn) {
        processors.push_back(eventFn);
    }

    void postEvent(const Event &event) {
        std::lock_guard<std::mutex> guard(pendingEventsMutex);
        pendingEvents.push_back(event);
    }

    void processOldestEvent() {
        if (pendingEvents.empty()) {
            return;
        }

        Event event = grabNextEvent();

        std::vector<std::thread> jobs;

        // launch one thread per event processor
        for (const EventFn &processor : processors) {
            jobs.push_back(std::thread(processor, event));
        }

        // wait for all of them to finish
        for (std::thread &job : jobs) {
            job.join();
        }
    }

private:
    Event grabNextEvent() {
        std::lock_guard<std::mutex> guard(pendingEventsMutex);
        Event event = pendingEvents.front();
        pendingEvents.erase(pendingEvents.begin());

        return event;
    }

    std::mutex pendingEventsMutex;
    std::vector<Event> pendingEvents;
    std::vector<EventFn> processors;
};

int main() {
    std::atomic<int> count = 0;
    std::atomic<bool> done = false;

    EventBus eventBus;

    eventBus.registerProcessor([](Event evt) {
        std::thread::id threadId = std::this_thread::get_id();

        logger(threadId, "Event " + evt.type);
        
        for (const auto &pair : evt.payload) {
            logger(threadId, "    " + pair.first + " " + pair.second);
        }
    });

    eventBus.registerProcessor([&count, &eventBus](Event evt) {
        if (evt.type == EVENT_COUNT_INCREASE) {
            count++;
            eventBus.postEvent({EVENT_COUNT_INCREASED});
        }
    });

    eventBus.registerProcessor([&count, &eventBus](Event evt) {
        if (evt.type == EVENT_COUNT_INCREASED && count >= 10) {
            eventBus.postEvent({EVENT_APP_TERMINATE});
        }
    });

    eventBus.registerProcessor([&done](Event evt) {
        if (evt.type == EVENT_APP_TERMINATE) {
            done = true;
        }
    });

    // this thread will continuouslys produce events until it gets the signal
    // feels hacky, as we are managing outside of the EventBus
    std::thread producer([&eventBus, &count, &done](){
        while(!done) {
            EventPayload payload;
            payload["count"] = std::to_string(count);

            eventBus.postEvent({EVENT_COUNT_INCREASE, payload});
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    while (!done) {
        eventBus.processOldestEvent();
    }

    producer.join();

    return 0;
}
