
#include "GraphicsDeviceGL.h"

#include "gl.h"

#include "Conversion.h"
#include "SubsetGL.h"
#include "UtilGL.h"

#include <iostream>
#include <xe/graphics/Material.h>
#include <xe/graphics/Subset.h>
#include <xe/graphics/Uniform.h>

namespace xe {
    static std::string hexstr(const GLenum value) {
        std::string str;
        str.resize(16, ' ');
        std::snprintf(str.data(), str.size(), "%x", value);

        return str;
    }

    static std::string stringval(const GLenum err) {
        switch (err) {
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_CONTEXT_LOST:
            return "GL_CONTEXT_LOST";

#if defined(GL_TABLE_TOO_LARGE)
        case GL_TABLE_TOO_LARGE:
            return "GL_TABLE_TOO_LARGE";
#endif

        default:
            return "UNKNOWN_ERR_CODE_" + hexstr(err);
        }
    }

    void GraphicsDeviceGL_callback(const char *name, void *, int, ...) {
        if (std::string(name) == "glGetError") {
            return;
        }

        GLenum err = glGetError();

        if (err != GL_NO_ERROR) {
            std::cerr << "GraphicsDeviceGL: Error while calling function " << name << '\n';
            std::cerr << "GraphicsDeviceGL: Errors generated:" << '\n';

            while (err != GL_NO_ERROR) {
                std::cerr << "GraphicsDeviceGL:" << stringval(err) << '\n';
                err = glGetError();
            }

            throw std::runtime_error("GraphicsDeviceGL: Error while calling function " + std::string(name));
        }
    }

    GraphicsDeviceGL::GraphicsDeviceGL(GraphicsContext *context) : context(context) {
        assert(context);

        std::cout << "[GL] Loading OpenGL Extensions ..." << '\n';
        glazeLoadFunctions(reinterpret_cast<GLAZE_GETPROCADDRESS>(context->getProcAddressFunctionGL()));

#if defined(GLAZE_DEBUG)
        glazeSetPostCallback(GraphicsDeviceGL_callback);
#endif
    }

    GraphicsDeviceGL::~GraphicsDeviceGL() {
    }

    Subset *GraphicsDeviceGL::createSubset(const SubsetDescriptor &desc) {
        return nullptr;
    }

    Buffer *GraphicsDeviceGL::createBuffer(const BufferDescriptor & /*desc*/) {
        return nullptr;
    }

    Program *GraphicsDeviceGL::createProgram(const ProgramDescriptor &desc) {
        return nullptr;
    }

    void GraphicsDeviceGL::draw(const Subset *subset, const SubsetEnvelope *envelopes, const size_t envelopeCount) {
        assert(subset);
        assert(envelopes);
        assert(envelopeCount > 0);

        auto subsetGL = dynamic_cast<const SubsetGL *>(subset);

        glBindVertexArray(subsetGL->getID());

        auto indexBuffer = subsetGL->getIndexBuffer();

        if (!indexBuffer) {
            for (size_t i = 0; i < envelopeCount; i++) {
                const SubsetEnvelope &env = envelopes[i];
                // glDrawArrays(primitiveGL, env.vertexStart, env.vertexCount);
            }
        } else {
            // TODO: Obtain dynamically the index data-type
            const GLenum indexTypeGL = GL_UNSIGNED_INT;

            for (size_t i = 0; i < envelopeCount; i++) {
                const SubsetEnvelope &env = envelopes[i];
                /*const GLenum primitiveGL = convertToGL(env.primitive);

                if (env.vertexStart == 0) {
                    glDrawElements(primitiveGL, env.vertexCount, indexTypeGL, nullptr);
                } else {
                    glDrawElementsBaseVertex(primitiveGL, env.vertexCount, indexTypeGL, nullptr, env.vertexStart);
                }
                */
            }
        }

        glBindVertexArray(0);
    }

    void GraphicsDeviceGL::beginFrame(const ClearFlags flags, const Vector4 &color, const float depth, const int stencil) {
        GLenum clearFlagsGL = 0;

        if (flags & ClearFlags::Color) {
            clearFlagsGL |= GL_COLOR_BUFFER_BIT;
            glClearColor(color.x, color.y, color.z, color.w);
        }

        if (flags & ClearFlags::Depth) {
            clearFlagsGL |= GL_DEPTH_BUFFER_BIT;
            glClearDepth(static_cast<GLdouble>(depth));
        }

        if (flags & ClearFlags::Stencil) {
            clearFlagsGL |= GL_STENCIL_BUFFER_BIT;
            glClearStencil(stencil);
        }

        glClear(clearFlagsGL);
    }

    void GraphicsDeviceGL::endFrame() {
        glFlush();

        context->present();
    }

    void GraphicsDeviceGL::preRenderMaterial(const Material *material) {
        /*
        const auto &rs = material->renderState;

        if (rs.depthTest) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        const GLenum depthFuncGL = convertToGL(rs.depthFunc);
        glDepthFunc(depthFuncGL);

        const GLenum faceGL = convertToGL(rs.frontFace);
        glFrontFace(faceGL);

        if (rs.cullBackFace) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }

        glLineWidth(rs.lineWidth);

        if (rs.blendEnable) {
            glEnable(GL_BLEND);

            const GLenum sfactorGL = convertToGL(rs.blendSource);
            const GLenum dfactorGL = convertToGL(rs.blendDestination);
            glBlendFunc(sfactorGL, dfactorGL);
        } else {
            glDisable(GL_BLEND);
        }

        for (int i = 0; i < material->layerCount; i++) {
            const auto &layer = material->layers[i];

            if (!layer.texture) {
                continue;
            }

            // FIXME: This will cause segfaults if the real implementation isn't derived from the Texture/TextureBaseGL family
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convertToGL(layer.magFilter));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertToGL(layer.minFilter));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convertToGL(layer.wrapS));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convertToGL(layer.wrapT));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, convertToGL(layer.wrapR));
        }*/
    }

    void GraphicsDeviceGL::postRenderMaterial(const Material *material) {
        /*
        const auto &rs = material->renderState;

        if (rs.depthTest) {
            glDisable(GL_DEPTH_TEST);
        }

        if (rs.cullBackFace) {
            glDisable(GL_CULL_FACE);
        }

        if (rs.blendEnable) {
            glDisable(GL_BLEND);
        }

        for (int i = 0; i < material->layerCount; i++) {
            const auto &layer = material->layers[i];

            if (!layer.texture) {
                continue;
            }

            // FIXME: This will cause segfaults if the real implementation isn't derived from the Texture/TextureBaseGL family
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glActiveTexture(GL_TEXTURE0);
        */
    }

    void GraphicsDeviceGL::setMaterial(const Material *material) {
        if (m_material == material) {
            return;
        }

        if (m_material) {
            this->postRenderMaterial(m_material);
        }

        this->preRenderMaterial(material);

        m_material = material;
    }

    void GraphicsDeviceGL::setProgram(const Program *program) {
    }

    const Program *GraphicsDeviceGL::getProgram() const {
        return nullptr;
    }

    void GraphicsDeviceGL::applyUniform(const UniformMatrix *uniformMatrix, const size_t count, const void *data) {
        // TODO: Add support for matrix transposition
        assert(m_program);
        assert(uniformMatrix);
        assert(count > 0);
        assert(data);

        int offset = 0;
        const auto ptr = reinterpret_cast<const std::byte *>(data);

        for (size_t i = 0; i < count; i++) {
            const UniformMatrix *current = &uniformMatrix[i];
            const GLint location = 0;
            assert(location >= 0);

            const TypeEncoding te = static_cast<TypeEncoding>(current->type);
            const auto values = reinterpret_cast<const GLfloat *>(&ptr[offset]);
            const uint8_t cols = getTypeCols(te);
            const uint8_t rows = getTypeRows(te);

            if (cols == 2) {
                if (rows == 2) {
                    glUniformMatrix2fv(location, current->count, GL_TRUE, values);
                } else if (rows == 3) {
                    glUniformMatrix2x3fv(location, current->count, GL_TRUE, values);
                } else if (rows == 4) {
                    glUniformMatrix2x4fv(location, current->count, GL_TRUE, values);
                }
            } else if (cols == 3) {
                if (rows == 2) {
                    glUniformMatrix3x2fv(location, current->count, GL_TRUE, values);
                } else if (rows == 3) {
                    glUniformMatrix3fv(location, current->count, GL_TRUE, values);
                } else if (rows == 4) {
                    glUniformMatrix3x4fv(location, current->count, GL_TRUE, values);
                }
            } else if (cols == 4) {
                if (rows == 2) {
                    glUniformMatrix4x2fv(location, current->count, GL_TRUE, values);
                } else if (rows == 3) {
                    glUniformMatrix4x3fv(location, current->count, GL_TRUE, values);
                } else if (rows == 4) {
                    glUniformMatrix4fv(location, current->count, GL_TRUE, values);
                }
            } else {
                assert(false && "Supplied UniformMatrixShape isn't supported");
            }

            offset += static_cast<int>(getTotalSizeInBytes(te)) * current->count;
        }
    }

    void GraphicsDeviceGL::applyUniform(const Uniform *uniform, const size_t count, const void *data) {
        assert(m_program);
        assert(uniform);
        assert(count > 0);
        assert(data);
    }

    void GraphicsDeviceGL::setViewport(const Viewport &viewport) {
        GLint x = static_cast<GLint>(viewport.position.x);
        GLint y = static_cast<GLint>(viewport.position.y);

        GLint w = static_cast<GLint>(viewport.size.x);
        GLint h = static_cast<GLint>(viewport.size.y);

        glViewport(x, y, w, h);

        m_viewport = viewport;
    }

    Viewport GraphicsDeviceGL::getViewport() const {
        return m_viewport;
    }

    const Material *GraphicsDeviceGL::getMaterial() const {
        return m_material;
    }
} // namespace xe
