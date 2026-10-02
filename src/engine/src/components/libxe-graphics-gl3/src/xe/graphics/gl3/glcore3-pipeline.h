
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"

namespace xe {
    /**
     * @brief A resolved uniform block binding baked into a PipelineGL at creation time.
     *
     * createPipelineGL calls glGetUniformBlockIndex followed by glUniformBlockBinding for every
     * entry, so the GPU-side wiring is already in place by the time a pipeline is used. Kept on
     * the pipeline for diagnostics (allowing a debugger to inspect which binding points are
     * actually live) and so destroyPipeline has the information to tear down any future state
     * caches keyed on the pipeline.
     */
    struct UniformBlockBindingGL {
        //! Block name resolved against the shader program at pipeline-creation time.
        std::string blockName;

        //! Binding point assigned to this block (matches PipelineUniformBlock::bindingPoint).
        uint32_t bindingPoint = 0;

        //! Block index returned by glGetUniformBlockIndex, captured for diagnostics.
        uint32_t blockIndex = 0;
    };

    /**
     * @brief GL-side representation of a PipelineDescriptor.
     *
     * Holds the clear state and a non-owning reference to the linked program, plus the uniform
     * block bindings that were baked into the program at pipeline creation. The pipeline itself
     * does not own the program - ShaderHandle ownership stays with the shader pool.
     */
    struct PipelineGL {
        //! RGBA clear color applied at beginFrame (once beginFrame is wired up on this backend).
        xe::Vector4 clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        //! Mask selecting which buffers glClear will clear.
        gl::Flags<gl::ClearBufferMask> clearMask = gl::ClearBufferMask::eColorBufferBit;

        //! Raw GL id for the program this pipeline draws with. Not owning.
        gl::Program shaderProgram;

        //! Original shader handle, retained for debug validation of uniform-block rebinds.
        ShaderHandle shaderHandle;

        //! Uniform block bindings baked into the program by createPipelineGL.
        std::vector<UniformBlockBindingGL> uniformBlockBindings;
    };

    tl::expected<PipelineHandle, BackendError> createPipelineGL(RenderDeviceBackendContext *ctx, const PipelineDescriptor &desc);
    void destroyPipelineGL(RenderDeviceBackendContext *ctx, PipelineHandle handle);
} // namespace xe
