
#include <xe/Core.h>

#ifndef XE_VERSION_MAJOR
#define XE_VERSION_MAJOR 0
#endif

#ifndef XE_VERSION_MINOR
#define XE_VERSION_MINOR 0
#endif

#ifndef XE_VERSION_REVISION
#define XE_VERSION_REVISION 0
#endif

namespace xe {
    Version Core::GetVersion() const {
        return {XE_VERSION_MAJOR, XE_VERSION_MINOR, XE_VERSION_REVISION};
    }
} // namespace xe
