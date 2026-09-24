
#include "glcore3-context.h"

namespace xe {

    /**
     * @brief Compile a single shader stage. Always returns a valid glaze::Unique; compile status
     * lives on the shader itself and must be inspected by the caller (getShaderiv + getShaderInfoLog).
     */
    static glaze::Unique<gl::Shader> compileShader(gl::ShaderType type, const char *src) {
        auto shader = glaze::makeUnique<gl::Shader>(type);
        gl::shaderSource(shader, 1, &src, nullptr);
        gl::compileShader(shader);
        return shader;
    }

    /**
     * @brief Attach/link the given shaders into a fresh program. Always returns a valid glaze::Unique;
     * link status lives on the program itself and must be inspected by the caller
     * (getProgramiv + getProgramInfoLog).
     */
    static glaze::Unique<gl::Program> linkProgram(const std::vector<glaze::Unique<gl::Shader>> &shaders) {
        auto program = glaze::makeUnique<gl::Program>();

        for (auto const &s : shaders) {
            gl::attachShader(program, s);
        }

        gl::linkProgram(program);

        for (auto const &s : shaders) {
            gl::detachShader(program, s);
        }

        return program;
    }

    tl::expected<ShaderHandle, BackendError> createShaderProgramGL(RenderDeviceBackendContext *ctx, const ShaderProgramDescriptor &desc) {
        auto &shaderPrograms = glctx(ctx)->shaderPrograms;

        auto vs = compileShader(gl::ShaderType::eVertexShader, desc.glslVertexShader.c_str());
        if (!gl::getShaderiv(vs, gl::ShaderParameterName::eCompileStatus)) {
            return makeBackendError(BackendErrorCode::ShaderCompileFailed, gl::getShaderInfoLog(vs));
        }

        auto fs = compileShader(gl::ShaderType::eFragmentShader, desc.glslFragmentShader.c_str());
        if (!gl::getShaderiv(fs, gl::ShaderParameterName::eCompileStatus)) {
            return makeBackendError(BackendErrorCode::ShaderCompileFailed, gl::getShaderInfoLog(fs));
        }

        std::vector<glaze::Unique<gl::Shader>> shaders;
        shaders.push_back(std::move(vs));
        shaders.push_back(std::move(fs));

        auto program = linkProgram(shaders);
        if (!gl::getProgramiv(program, gl::ProgramProperty::eLinkStatus)) {
            return makeBackendError(BackendErrorCode::ShaderLinkFailed, gl::getProgramInfoLog(program));
        }

        if (!poolHasCapacity(shaderPrograms)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "shader program pool exhausted (16-bit index field)");
        }

        auto const [index, gen] = acquireSlot(shaderPrograms, ProgramGL{std::move(program)});
        return ShaderHandle::make(gen, index);
    }

    void destroyShaderProgramGL(RenderDeviceBackendContext *ctx, ShaderHandle handle) {
        auto &shaderPrograms = glctx(ctx)->shaderPrograms;
        uint32_t const index = handle.index();
        assert(index < shaderPrograms.size() && "destroyShaderProgramGL: handle index out of range");
        auto &slot = shaderPrograms[index];
        assert(slot.obj && "destroyShaderProgramGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyShaderProgramGL: stale handle (generation mismatch)");
        slot.obj.reset();
    }

} // namespace xe
