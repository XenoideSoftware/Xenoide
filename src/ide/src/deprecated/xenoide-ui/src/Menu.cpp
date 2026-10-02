
#include <xenoide/ui/Menu.h>

namespace xenoide {
    MenuData MenuData::menuBar(const std::vector<MenuData> &childs) {
        MenuData menu;

        menu.childs = childs;
        menu.kind = MenuKind::SubMenu;

        return menu;
    }

    MenuData MenuData::menu(const std::string &text, const std::vector<MenuData> &childs) {
        MenuData menu;

        menu.text = text;
        menu.childs = childs;
        menu.kind = MenuKind::SubMenu;

        return menu;
    }

    MenuData MenuData::action(const std::function<void()> &callback, const std::string &text, std::optional<ActionId> actionId, const Shortcut &shortcut) {
        MenuData menu;

        menu.callback = callback;
        menu.text = text;
        menu.shortcut = shortcut;
        menu.kind = MenuKind::Action;
        menu.actionId = actionId;

        return menu;
    }

    MenuData MenuData::separator() {
        MenuData menu;

        menu.kind = MenuKind::Separator;

        return menu;
    }
} // namespace xenoide
