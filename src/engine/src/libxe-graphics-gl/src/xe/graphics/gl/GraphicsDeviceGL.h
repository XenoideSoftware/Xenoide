
#ifndef __XE_GRAPHICS_GL_GRAPHICSDEVICEGL_HPP__
#define __XE_GRAPHICS_GL_GRAPHICSDEVICEGL_HPP__

#include <xe/graphics/GraphicsContext.h>
#include <xe/graphics/GraphicsDevice.h>
#include <xe/graphics/Viewport.h>

namespace xe {
    class ProgramGL;
    class SubsetGL;

    class GraphicsDeviceGL : public GraphicsDevice {
    public:
        explicit GraphicsDeviceGL(GraphicsContext *context);

        ~GraphicsDeviceGL();

        Subset *createSubset(const SubsetDescriptor &desc) override;

        Buffer *createBuffer(const BufferDescriptor &bufferDescriptor) override;

        Texture2D *createTexture2D(const PixelFormat format, const Vector2i &size, const PixelFormat sourceFormat, const DataType sourceDataType, const void *sourceData) override;

        Texture3D *createTexture3D(const PixelFormat format, const Vector3i &size, const PixelFormat sourceFormat, const DataType sourceDataType, const void *sourceData) override;

        Texture2DArray *createTexture2DArray(const PixelFormat format, const Vector2i &size, const int count) override;

        TextureCubeMap *
        createTextureCubeMap(const PixelFormat format, const Vector2i &size, const PixelFormat sourceFormat, const DataType sourceDataType, const void **sourceData) override;

        Program *createProgram(const ProgramDescriptor &programDescriptor) override;

        void setMaterial(const Material *material) override;

        const Material *getMaterial() const override;

        void setProgram(const Program *program) override;

        const Program *getProgram() const override;

        void applyUniform(const UniformMatrix *uniformMatrix, const size_t count, const void *data) override;

        void applyUniform(const Uniform *uniform, const size_t count, const void *data) override;

        void beginFrame(const ClearFlags flags, const Vector4 &color, const float depth, const int stencil) override;

        void draw(const Subset *subset, const SubsetEnvelope *envelopes, const size_t envelopeCount) override;

        void endFrame() override;

        void setViewport(const Viewport &viewport) override;

        Viewport getViewport() const override;

    private:
        void preRenderMaterial(const Material *material);

        void postRenderMaterial(const Material *material);

    private:
        GraphicsContext *context = nullptr;

        const ProgramGL *m_program = nullptr;
        const Material *m_material = nullptr;

        Viewport m_viewport;
    };
} // namespace xe

#endif
