
#include <xenoide/ui/ActionBus.h>

namespace xenoide {
    MessageHandler::MessageHandler(MessageBus *bus) : bus(bus) {
        bus->registerHandler(this);
    }

    MessageHandler::~MessageHandler() {
        bus->unregisterHandler(this);
    }
} // namespace xenoide

namespace xenoide {
    void MessageBus::registerHandler(MessageHandler *handler) {
        handlers.insert(handler);
    }

    void MessageBus::unregisterHandler(MessageHandler *handler) {
        handlers.erase(handler);
    }

    void MessageBus::postMessage(const MessageId &messageId, const std::string &data) {
        for (MessageHandler *handler : handlers) {
            handler->handle(messageId, data);
        }
    }
} // namespace xenoide

namespace xenoide {

    void createNewDocument() {
    }

} // namespace xenoide
