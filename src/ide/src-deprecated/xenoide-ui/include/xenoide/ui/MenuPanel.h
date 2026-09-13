
#ifndef __XENOIDE_UI_MENUPANEL_HPP__
#define __XENOIDE_UI_MENUPANEL_HPP__

#include <xenoide/core/Predef.h>

namespace xenoide {
    struct MenuData;

    /**
     * @brief View that shows to the user a MenuBar.
     */
    class MenuPanel {
    public:
        virtual ~MenuPanel();

        virtual void setupMenuBar(const MenuData &menu) = 0;
    };
}

#endif
