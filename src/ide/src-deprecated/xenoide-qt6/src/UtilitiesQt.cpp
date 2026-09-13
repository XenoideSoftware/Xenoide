
#include "UtilitiesQt.h"

#include "xenoide/ui/ActionBus.h"

#include "ActionBusQt.h"

#include <map>

namespace xenoide {
    static const std::map<Key, std::string> keyMap = {
        { Key::F1, "F1" },
        { Key::F2, "F2" },
        { Key::F3, "F3" },
        { Key::F4, "F4" },
        { Key::F5, "F5" },
        { Key::F6, "F6" },
        { Key::F7, "F7" },
        { Key::F8, "F8" },
        { Key::F9, "F9" },
        { Key::F10, "F10" },
        { Key::F11, "F11" },
        { Key::F12, "F12" },
        { Key::Q, "Q" },
        { Key::W, "W" },
        { Key::E, "E" },
        { Key::R, "R" },
        { Key::T, "T" },
        { Key::Y, "Y" },
        { Key::U, "U" },
        { Key::I, "I" },
        { Key::O, "O" },
        { Key::P, "P" },
        { Key::A, "A" },
        { Key::S, "S" },
        { Key::D, "D" },
        { Key::F, "F" },
        { Key::G, "G" },
        { Key::H, "H" },
        { Key::J, "J" },
        { Key::K, "K" },
        { Key::L, "L" },
        { Key::Z, "Z" },
        { Key::X, "X" },
        { Key::C, "C" },
        { Key::V, "V" },
        { Key::B, "B" },
        { Key::N, "N" },
        { Key::M, "M" }
    };

    static const std::map<Modifier, std::string> modifierMap = {
        { Modifier::Ctrl, "Ctrl" },
        { Modifier::Shift, "Shift" },
        { Modifier::Alt, "Alt" },
        { Modifier::CtrlShift, "Ctrl+Shift" },
        { Modifier::CtrlShiftAlt, "Ctrl+Shift+Alt" },
        { Modifier::ShiftAlt, "Shift+Alt" }
    };

    QKeySequence mapShortcut(const Shortcut &shortcut) {
        if (shortcut.isNone()) {
            return QKeySequence();
        }

        const std::string key = keyMap.find(shortcut.key)->second;
        
        if (shortcut.modifier != Modifier::None) {
            const std::string modifier = modifierMap.find(shortcut.modifier)->second;

            return QKeySequence((modifier + "+" + key).c_str());
        } else {
            return QKeySequence(key.c_str());
        }
    }

    void setupMenu(QMenu *parentMenuPtr, const Menu &menu, ActionBusQt *actionBus) {
        switch (menu.kind) {
            case MenuKind::SubMenu: {
                QMenu *menuPtr = parentMenuPtr->addMenu(menu.text.c_str());

                for (const Menu &childMenu : menu.childs) {
                    setupMenu(menuPtr, childMenu, actionBus);
                }

                break;
            }

            case MenuKind::Action: {
                auto action = parentMenuPtr->addAction(menu.text.c_str(), nullptr, nullptr, mapShortcut(menu.shortcut));

                if (actionBus && menu.actionId.has_value()) {
                    actionBus->registerAction(menu.actionId.value(), action);
                }

                if (menu.callback) {
                    QObject::connect(action, &QAction::triggered, menu.callback);
                }

                break;
            }

            case MenuKind::Separator: {
                parentMenuPtr->addSeparator();
                break;
            }
        }
    }

    QMenuBar* createMenuBar(QWidget *parent, const Menu &menuBar, ActionBusQt *actionBus) {
        auto menuBarPtr = new QMenuBar(parent);

        for (const Menu &menu : menuBar.childs) {
            QMenu *menuPtr = menuBarPtr->addMenu(menu.text.c_str());

            for (const Menu &childMenu : menu.childs) {
                setupMenu(menuPtr, childMenu, actionBus);
            }
        }

        return menuBarPtr;
    }
}
