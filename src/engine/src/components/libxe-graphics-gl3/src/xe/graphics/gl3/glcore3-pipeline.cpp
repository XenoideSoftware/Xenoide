
#include "glcore3-context.h"

namespace xe {

    tl::expected<PipelineHandle, BackendError> createPipelineGL(RenderDeviceBackendContext *ctx, const PipelineDescriptor &desc) {
        auto &pipelines = glctx(ctx)->pipelines;
        auto &shaderPrograms = glctx(ctx)->shaderPrograms;

        if (!poolHasCapacity(pipelines)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "pipeline pool exhausted (16-bit index field)");
        }

        const ProgramGL *programPtr = tryProgramExtract(shaderPrograms, desc.shaderProgramHandle);
        if (programPtr == nullptr) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "createPipelineGL: shaderProgramHandle is invalid or references a freed shader program");
        }

        PipelineGL pipeline;
        pipeline.clearColor = desc.clearColor;
        pipeline.shaderProgram = programPtr->program.get();
        pipeline.shaderHandle = desc.shaderProgramHandle;
        pipeline.uniformBlockBindings.reserve(desc.uniformBlocks.size());

        for (const PipelineUniformBlock &block : desc.uniformBlocks) {
            GLuint const blockIndex = gl::getUniformBlockIndex(pipeline.shaderProgram, block.blockName.c_str());
            if (blockIndex == GL_INVALID_INDEX) {
                return makeBackendError(
                    BackendErrorCode::InvalidDescriptor,
                    std::string{"createPipelineGL: uniform block '"} + block.blockName + "' is not active in the shader program"
                );
            }

            gl::uniformBlockBinding(pipeline.shaderProgram, blockIndex, block.bindingPoint);
            pipeline.uniformBlockBindings.push_back({block.blockName, block.bindingPoint, blockIndex});
        }

        auto const [index, gen] = acquireSlot(pipelines, std::move(pipeline));
        return PipelineHandle::make(gen, index);
    }

    void destroyPipelineGL(RenderDeviceBackendContext *ctx, PipelineHandle handle) {
        auto &pipelines = glctx(ctx)->pipelines;
        uint32_t const index = handle.index();
        assert(index < pipelines.size() && "destroyPipelineGL: handle index out of range");
        auto &slot = pipelines[index];
        assert(slot.obj && "destroyPipelineGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyPipelineGL: stale handle (generation mismatch)");
        slot.obj.reset();
    }
} // namespace xe
