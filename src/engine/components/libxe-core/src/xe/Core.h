
#ifndef __XE_CORE_HPP__
#define __XE_CORE_HPP__

#include <xe/Predef.h>

namespace xe {
    struct Version {
        int major, minor, revision;
    };

    /**
     * @brief The Core Root class
     */
    class XE_API Core {
    public:
        Version GetVersion() const;
    };
} // namespace xe

#endif
