#pragma once

#include <xenoide/ui/ActionBus.h>

#include <QAction>
#include <map>

namespace xenoide {

    class ActionBusQt : public ActionBus {
    public:
        void registerAction(ActionId id, QAction *action);

    protected:
        void sendAction(ActionId id) override;

    private:
        std::map<ActionId, QAction *> actionMap;
    };

} // namespace xenoide
