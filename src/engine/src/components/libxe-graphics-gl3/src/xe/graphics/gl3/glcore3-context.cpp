
#include "glcore3-context.h"

namespace xe {
    tl::expected<RenderDeviceBackendContext *, BackendError> createContextGL() {
        auto *ctx = new (std::nothrow) RenderDeviceBackendContextGL();
        if (!ctx) {
            return makeBackendError(BackendErrorCode::AllocationFailed, "failed to allocate RenderDeviceBackendContextGL");
        }
        return ctx;
    }

    void destroyContextGL(RenderDeviceBackendContext *ctx) {
        delete glctx(ctx);
    }
} // namespace xe
