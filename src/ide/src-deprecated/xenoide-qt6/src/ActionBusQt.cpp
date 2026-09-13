
#include "ActionBusQt.h"

#include <iostream>

namespace xenoide {
  void ActionBusQt::registerAction(ActionId id, QAction* action) {
    actionMap[id] = action;
  }

  void ActionBusQt::sendAction(ActionId actionId) {
    const auto it = actionMap.find(actionId);

    if (it == actionMap.end()) {
      std::cout << "No such action " << actionId << std::endl;
      return;
    }

    std::cout << "Triggering action for " << actionId << std::endl;

    QAction* action = it->second;
    action->trigger();
  }
}
