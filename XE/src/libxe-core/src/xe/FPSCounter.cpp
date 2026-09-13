//
// Created by Felipe Apablaza on 26-03-23.
//

#include "FPSCounter.h"

namespace xe {
    bool FPSCounter::frame() {
        const int milliseconds = xe::Timer::getTick() - lastTime;

        if (milliseconds < 1000) {
            currentFrames++;

            return false;
        } else {
            fps = currentFrames;
            currentFrames = 0;
            lastTime = xe::Timer::getTick();

            return true;
        }
    }
} // namespace xe
