
#ifndef __XENOIDE_UI_QT5_UTILITIESQT_HPP__
#define __XENOIDE_UI_QT5_UTILITIESQT_HPP__

#include <QMenuBar>
#include <xenoide/ui/Menu.h>

namespace xenoide {
    class ActionBusQt;

    extern void setupMenu(QMenu *parentMenuPtr, const Menu &menu, ActionBusQt *actionBus = nullptr);

    extern QMenuBar* createMenuBar(QWidget *parent, const Menu &menuBar, ActionBusQt *actionBus = nullptr);
}

#endif
