
#include <xe/Timer.h>

#if defined(XE_OS_WINDOWS)
#include <Windows.h>

namespace xe {
    uint32_t Timer::getTick() {
        const DWORD dwTickCount = ::GetTickCount();
        return dwTickCount;
    }
} // namespace xe

#elif defined(__APPLE__)
#include <cassert>
#include <mach/mach_time.h>

// Returns monotonic time in nanos, measured from the first time the function
// is called in the process.
static uint64_t monotonicTimeNanos() {
    uint64_t now = mach_absolute_time();

    static struct Data {
        static uint64_t scaleHighPrecision(uint64_t i, uint32_t numer, uint32_t denom) {
            uint64_t high = (i >> 32) * numer;
            uint64_t low = (i & 0xffffffffull) * numer / denom;
            uint64_t highRem = ((high % denom) << 32) / denom;
            high /= denom;
            return (high << 32) + highRem + low;
        }

        Data(uint64_t bias_) : bias(bias_), mtiStatus(mach_timebase_info(&tb)) {
        }

        uint64_t scale(uint64_t i) {
            return scaleHighPrecision(i - bias, tb.numer, tb.denom);
        }

        mach_timebase_info_data_t tb{};
        uint64_t bias;
        kern_return_t mtiStatus;

        bool valid() const {
            return mtiStatus == KERN_SUCCESS;
        }

    } data(now);

    return data.scale(now);
}

namespace xe {
    uint32_t Timer::getTick() {
        const uint64_t nanos = monotonicTimeNanos();
        return nanos / 1000000;
    }
} // namespace xe

#elif defined(__linux__)
#include <time.h>

namespace xe {
    uint32_t Timer::getTick() {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        return 1000 * ts.tv_sec + ts.tv_nsec / 1000000;
    }
} // namespace xe

#else
#error "Unsupported platform"
#endif
