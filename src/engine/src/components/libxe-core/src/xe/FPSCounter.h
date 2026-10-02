
#pragma once

#ifndef XE_FPSCOUNTER_H
#define XE_FPSCOUNTER_H

#include "Timer.h"

namespace xe {
    class FPSCounter {
    public:
        //! must be called once per frame
        bool frame();

        int getFPS() const {
            return fps;
        }

    private:
        int lastTime = xe::Timer::getTick();
        int currentFrames = 0;
        int fps = 0;
    };
} // namespace xe

#endif // XE_FPSCOUNTER_H
