
#ifndef __XENOIDE_UI_MENU_HPP__
#define __XENOIDE_UI_MENU_HPP__

#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace xenoide {
    enum class MenuKind {
        SubMenu,
        Action,
        Separator
    };

    enum class Modifier {
        None = 0,
        Ctrl = 1,
        Shift = 2, 
        Alt = 4,
        CtrlShift = Ctrl + Shift,
        CtrlShiftAlt = Ctrl + Shift + Alt,
        ShiftAlt = Shift + Alt
    };

    inline bool operator& (const Modifier &modifiers, const Modifier &modifier) {
        return static_cast<int>(modifiers) & static_cast<int>(modifier);
    }

    enum class Key {
        None = 0,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        Q, W, E, R, T, Y, U, I, O, P,
        A, S, D, F, G, H, J, K, L,
        Z, X, C, V, B, N, M
    };

    struct Shortcut {
        Modifier modifier = Modifier::None;
        Key key = Key::None;

        Shortcut() {}

        Shortcut(const Key key) {
            this->key = key;
        }

        Shortcut(const Modifier modifier, const Key key) {
            this->modifier = modifier;
            this->key = key;
        }

        bool isNone() const {
            return key == Key::None;
        }
    };

    enum class ActionId;

    // TODO: We should decouple the action-menu relationship
    // A map between Action and "callbacks" is one thing.
    // Menu hierarchies (and evential toolbars and other controls that reference them) are other UI object
    struct MenuData {
        std::string text;
        Shortcut shortcut;
        std::vector<MenuData> childs;
        MenuKind kind;
        std::function<void()> callback;
        std::optional<ActionId> actionId;

        static MenuData menuBar(const std::vector<MenuData> &childs);

        static MenuData menu(const std::string &text, const std::vector<MenuData> &childs);

        static MenuData action(const std::function<void()> &callback, const std::string &text, std::optional<ActionId> actionId = {}, const Shortcut &shortcut = {});

        static MenuData separator();
    };

    using Menu = MenuData;
}

#endif
