
#pragma once

#include <ostream>
#include <set>
#include <array>

namespace xenoide {

enum class ActionId {
  DocumentNew,
  DocumentOpen,
  DocumentSave,
  DocumentSaveAll,
  DocumentClose,
  FolderOpen,
  AppExit,
  ShowOpenDocumentDialog,
  ShowSaveDocumentDialog,
  ShowOpenFolderDialog,
  ShowAboutDialog,

  // add new elements here
  // don't forget to update to_string as well
};

/**
 * @brief Application-wide event bus
 */
class ActionBus {
public:
  virtual ~ActionBus() = default;

  // Allows Presenters to send events, without having references to the other presenters
  // these events are supported by the native UI system
  virtual void sendAction(ActionId action) = 0;
};

using MessageId = ActionId;

class MessageBus;
class MessageHandler {
public:
  explicit MessageHandler(MessageBus* bus);

  virtual ~MessageHandler();

  virtual void handle(const MessageId &messageId, const std::string &data) = 0;

protected:
  MessageBus* bus = nullptr;
};

class MessageBus {
  friend class MessageHandler;

  void registerHandler(MessageHandler* handler);

  void unregisterHandler(MessageHandler* handler);

public:
  void postMessage(const MessageId &messageId, const std::string &data);

private:
  std::set<MessageHandler*> handlers;
};

}

inline std::string_view to_string(const xenoide::ActionId &id) {
  constexpr std::array<std::string_view, 11> actionIdNames  {
    "DocumentNew",
    "DocumentOpen",
    "DocumentSave",
    "DocumentSaveAll",
    "DocumentClose",
    "FolderOpen",
    "AppExit",
    "ShowOpenDocumentDialog",
    "ShowSaveDocumentDialog",
    "ShowOpenFolderDialog",
    "ShowAboutDialog",
  };

  if (const auto i = static_cast<int>(id); i >= 0 && i < actionIdNames.size()) {
    return actionIdNames[i];
  }

  return "UnknownActionId";
}

inline std::ostream& operator<<(std::ostream &os, const xenoide::ActionId& id) {
  return os << to_string(id);
}
