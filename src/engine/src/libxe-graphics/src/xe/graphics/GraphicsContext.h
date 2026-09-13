
#pragma once

#include <set>
#include <tuple>

#include "GraphicsAPI.h"
#include "PixelFormat.h"

namespace xe {
    class GraphicsContext {
    public:
        struct Descriptor {
            //! Requested Backend.
            GraphicsBackend backend = GraphicsBackend::Auto;

            PixelFormat frameBufferFormat = PixelFormat::R8G8B8A8;
            DepthFormat depthBufferFormat = DepthFormat::D24;
            StencilFormat stencilBufferFormat = StencilFormat::SUnused;
        };

    public:
        using ProcAddressGL = void (*)();
        using GetProcAddressGL = ProcAddressGL (*)(const char *);

    public:
        virtual ~GraphicsContext() {
        }

        virtual GetProcAddressGL getProcAddressFunctionGL() const = 0;

        virtual void present() = 0;

        virtual Descriptor getRequestedDescriptor() const = 0;

        virtual Descriptor getDescriptor() const = 0;
    };
} // namespace xe
