
#include "Renderer.h"

#include "../../../../apostate/src/apostate/Platform.h"
#include "xe/Logger.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include <glaze/gl.h>

#if 0

namespace gl {
    class ErrorChecker {
    public:
        ErrorChecker() { check(); }
        ~ErrorChecker() { check(); }

    private:
        void check() const {
#if defined(GLHPP_ERRORCHECKING)
            const GLenum error = glGetError();

            if (error != GL_NO_ERROR) {
                std::cerr << "GL Error Detected:\t" << GLErrorToString(error) << "\n"

                abort();
            }
#endif
        }
    };

    template<typename Enum, typename Type>
    class Mask {
    public:
        Mask() {}

        Mask(const Enum value) : mValue(value) {}

        Mask<Enum, Type> operator| (const Mask<Enum, Type> &rhs) const {
            return Mask<Enum, Type>(mValue | rhs.mValue);
        }

        Type value() const {
            return mValue;
        }

    private:
        Type mValue = static_cast<Type>(0);
    };

    enum class ClearFlags : GLbitfield {
        eColorBufferBit = GL_COLOR_BUFFER_BIT,
        eDepthBufferBit = GL_DEPTH_BUFFER_BIT,
        eStencilBufferBit = GL_STENCIL_BUFFER_BIT
    };

    enum class ShaderType : GLenum {
        eVertexShader = GL_VERTEX_SHADER,
        eFragmentShader = GL_FRAGMENT_SHADER,
    };

    Mask<ClearFlags, GLbitfield> operator| (const ClearFlags &value1, const ClearFlags &value2) {
        return Mask<ClearFlags, GLbitfield>(value1 | value2);
    }

    /**
     * @brief clear buffers to preset values
     * @param mask Bitwise OR of masks that indicate the buffers to be cleared. The three masks are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, and GL_STENCIL_BUFFER_BIT.
     */
    void clear(const Mask<ClearFlags, GLbitfield> mask) {
        ErrorChecker checker;
        glClear(mask.value());
    }

    void flush() {
        ErrorChecker checker;
        glFlush();
    }

    template<typename Type, typename Tag>
    class Resource {
    public:
        Resource() {}

        explicit Resource(const GLuint resource) : resource{resource} {
            assert(resource);
        }

        GLuint value() const {
            return resource;
        }

        operator bool() const {
            return resource != 0;
        }

    private:
        GLuint resource = 0;
    };

    struct shader_tag {};

    using Shader = Resource<GLuint, shader_tag>;

    Shader createShader(const ShaderType type) {
        ErrorChecker checker;
        return Shader{glCreateShader(static_cast<GLenum>(type))};
    }

    void shaderSource (const Shader shader, const GLsizei count, const GLchar *const*string, const GLint *length) {
        assert(string);
        assert(length);
        assert(count > 0);

        ErrorChecker checker;
        glShaderSource(shader.value(), count, string, length);
    }

    void compileShader(const Shader shader) {
        ErrorChecker checker;
        glCompileShader(shader.value());
    }

    void deleteShader(const Shader shader) {
        glDeleteShader(shader.value());
    }
}


namespace test {
    gl::Shader createShader(const std::string &source, const gl::ShaderType type) {
        const GLchar * const sources = source.c_str();
        const GLint sourceSizes = source.size();

        gl::Shader shader = gl::createShader(type);
        gl::shaderSource(shader, 1, &sources, &sourceSizes);
        gl::compileShader(shader);

        GLint status = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE) {
            char buffer[2048] = {};
            GLsizei size = 0;

            glGetShaderInfoLog(shader, 2048, &size, buffer);
        }
    }

    void renderFrame() {
        gl::clear(gl::ClearFlags::eColorBufferBit | gl::ClearFlags::eDepthBufferBit);

        gl::flush();
    }
}

#endif

static std::string GLErrorToString(GLenum error) {
    switch (error) {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "<Unknown Error Value: " + std::to_string(error) + ">";
    }
}

#ifndef NDEBUG
#define M_Assert(Expr, Msg) __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#define M_Assert(Expr, Msg) ;
#endif

void __M_Assert(const char *expr_str, bool expr, const char *file, int line, const char *msg) {
    if (!expr) {
        std::cerr << "Assert failed:\t" << msg << "\n"
                  << "Expected:\t" << expr_str << "\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}

struct GLErrorRAII {
    GLErrorRAII(const char *file, const int line) : file(file), line(line) {
        check();
    }

    ~GLErrorRAII() {
        check();
    }

    void check() const {
        const GLenum error = glGetError();

        if (error != GL_NO_ERROR) {
            std::cerr << "GL Error Detected:\t" << GLErrorToString(error) << "\n"
                      << "Source:\t\t" << file << ", line " << line << "\n";

            abort();
        }
    }

    const char *file;
    const int line;
};

#define GL_SCOPED_ERROR_CHECK() GLErrorRAII __gl_error_raii(__FILE__, __LINE__)

#if defined(GLAZE_DEBUG)
void pre_call_callback_gl(const char *name, void *funcptr, int len_args, ...) {
    (void)name;
    (void)funcptr;
    (void)len_args;

    if (strcmp(name, "glLinkProgram") == 0) {
        std::cout << "pre glLinkProgram" << '\n';
    }
}

void post_call_callback_gl(const char *name, void *funcptr, int len_args, ...) {
    (void)funcptr;

    const GLenum error_code = glaze_glGetError();

    if (error_code == GL_NO_ERROR) {
        return;
    }

    va_list valist{};
    va_start(valist, len_args);

    std::cerr << name << "(";

    // TODO: Generate a lookup-table, for Debug builds, to process this debug information.
    if (strcmp(name, "glEnableVertexAttribArray") == 0) {
        for (int i = 0; i < len_args; i++) {
            std::cerr << va_arg(valist, GLuint);

            if (i < len_args - 1) {
                std::cerr << ", ";
            }
        }
    } else {
        std::cerr << "<unknown function arguments>";
    }

    std::cerr << ")" << '\n';
    std::cerr << "Error: The previous command couldn't be completed, due to the error " << GLErrorToString(error_code) << '\n';

    va_end(valist);

    abort();
}
#endif

Renderer::Renderer(Platform &platform) : platform{platform} {
}

bool Renderer::initialize() {
    glazeLoadFunctions(reinterpret_cast<GLAZE_GETPROCADDRESS>(platform.getGLProcAddressProcedure()));

    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

    GLint extensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

    std::cout << "Glaze - Initialized extensions (" << extensionCount << "): " << '\n';

    for (int i = 0; i < extensionCount; i++) {
        std::cout << "    " << glGetStringi(GL_EXTENSIONS, i) << '\n';
    }

#if defined(GLAZE_DEBUG)
    glazeSetPreCallback(pre_call_callback_gl);
    glazeSetPostCallback(post_call_callback_gl);
#endif

    return true;
}

Renderer::~Renderer() {
}

GLuint Renderer::createShader(const std::string &source, const GLenum type) {
    GLuint shader = glCreateShader(type);

    const GLchar *const sources = source.c_str();
    const GLint sourceSizes = static_cast<GLint>(source.size());

    glShaderSource(shader, 1, &sources, &sourceSizes);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[2048] = {};
        GLsizei size = 0;

        glGetShaderInfoLog(shader, 2048, &size, buffer);
        XE_LOG_ERROR("Error while compiling shader: {}\n", std::string(buffer));

        return 0;
    }

    return shader;
}

GLuint Renderer::createShaderProgram(const std::vector<GLuint> &shaders) {
    GLuint programId = glCreateProgram();

    for (const GLuint shaderId : shaders) {
        assert(shaderId);
        assert(glIsShader(shaderId));

        glAttachShader(programId, shaderId);
    }

    glLinkProgram(programId);

    GLint status = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[2048] = {};
        GLsizei size = 0;

        glGetProgramInfoLog(programId, 2048, &size, buffer);
        const std::string msg = buffer;
        std::cerr << "Error: " << msg << '\n';

        return 0;
    }

    return programId;
}

GLuint Renderer::createBuffer(const GLenum target, const GLsizeiptr size, const void *data, GLenum usage) {
    assert(size);
    assert(data);

    GLuint buffer = 0;

    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);

    assert(glGetError() == GL_NO_ERROR);

    return buffer;
}

ShaderLocationMap Renderer::createShaderLocationMap(const GLuint programId) {
    assert(programId);
    assert(glIsProgram(programId));

    ShaderLocationMap location;

    location.coord = 0;
    assert(location.coord >= 0);

    location.normal = 1;
    assert(location.normal >= 0);

    location.texCoord = 2;
    assert(location.texCoord >= 0);

    location.uModel = glGetUniformLocation(programId, "uModel");
    location.uView = glGetUniformLocation(programId, "uView");
    location.uProj = glGetUniformLocation(programId, "uProj");

    location.uMaterialDiffuseSamplerEnable = glGetUniformLocation(programId, "uMaterialDiffuseSamplerEnable");
    location.uMaterialDiffuseSampler = glGetUniformLocation(programId, "uMaterialDiffuseSampler");
    location.uMaterialAmbient = glGetUniformLocation(programId, "uMaterialAmbient");
    location.uMaterialDiffuse = glGetUniformLocation(programId, "uMaterialDiffuse");
    location.uMaterialSpecular = glGetUniformLocation(programId, "uMaterialSpecular");

    location.uLightAmbient = glGetUniformLocation(programId, "uLightAmbient");
    location.uLightDirection = glGetUniformLocation(programId, "uLightDirection");
    location.uLightDiffuse = glGetUniformLocation(programId, "uLightDiffuse");

    return location;
}

void Renderer::beginRenderFrame() {
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(program);
}

void Renderer::endRenderFrame() {
    glFlush();
    platform.swapBuffers();
}

Mesh Renderer::createMeshVAO(const ShaderLocationMap &location, const MeshData &meshData) {
    Mesh meshVAO;

    meshVAO.material = meshData.materialIndex;

    GLuint coordBuffer = createBuffer(GL_ARRAY_BUFFER, meshData.vertexCoord, GL_STATIC_DRAW);

    GLuint normalBuffer = 0;
    if (!meshData.vertexNormal.empty()) {
        normalBuffer = createBuffer(GL_ARRAY_BUFFER, meshData.vertexNormal, GL_STATIC_DRAW);
    }

    GLuint texCoordBuffer = 0;
    if (!meshData.vertexTexCoord.empty()) {
        texCoordBuffer = createBuffer(GL_ARRAY_BUFFER, meshData.vertexTexCoord, GL_STATIC_DRAW);
    }

    GLuint indexBuffer = 0;
    if (!meshData.indices.empty()) {
        indexBuffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.indices, GL_STATIC_DRAW);

        meshVAO.indexDataType = GL_UNSIGNED_INT;
        meshVAO.indexed = true;
        meshVAO.count = meshData.indexCount;
        meshVAO.primitiveType = GL_TRIANGLES;
    } else {
        meshVAO.indexDataType = GL_UNSIGNED_INT;
        meshVAO.indexed = false;
        meshVAO.count = meshData.vertexCount;
        meshVAO.primitiveType = GL_TRIANGLES;
    }

    glGenVertexArrays(1, &meshVAO.vao);
    glBindVertexArray(meshVAO.vao);

    glEnableVertexAttribArray(location.coord);
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    glVertexAttribPointer(location.coord, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (normalBuffer) {
        assert(location.normal >= 0);
        glEnableVertexAttribArray(location.normal);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(location.normal, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (texCoordBuffer) {
        assert(location.texCoord >= 0);

        glEnableVertexAttribArray(location.texCoord);
        glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
        glVertexAttribPointer(location.texCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (indexBuffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    }

    glBindVertexArray(0);

    return meshVAO;
}

GLuint Renderer::createTexture(
    GLenum internalFormat, const unsigned width, const unsigned height, const GLenum format, const GLenum type, const void *data, const GLuint wrapS, const GLuint wrapT
) {
    // Generate a new texture
    GLuint texture = 0;
    glGenTextures(1, &texture);

    // Bind the texture to a name
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

    // Set texture interpolation method to use linear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

GLuint Renderer::createTexture(GLenum internalFormat, const unsigned width, const unsigned height, const GLenum format, const GLenum type, const void *data) {
    return createTexture(internalFormat, width, height, format, type, data, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void Renderer::renderCamera(const ShaderLocationMap &location, const Camera &camera) {
    const glm::mat4 proj = glm::perspective(camera.fov, camera.aspect, camera.znear, camera.zfar);

    const glm::mat4 view = glm::lookAt(camera.position, camera.lookAt, camera.up);

    glUniformMatrix4fv(location.uProj, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(location.uView, 1, GL_FALSE, glm::value_ptr(view));
}

void Renderer::renderLighting(const GLuint programId, const Lighting &lighting) {
    GLint loc = 0;

    loc = glGetUniformLocation(programId, "uEnableLighting");
    assert(loc >= 0);
    glUniform1i(loc, lighting.enabled == true ? 1 : 0);

    loc = glGetUniformLocation(programId, "uLighting.globalAmbient");
    assert(loc >= 0);
    glUniform4fv(loc, 1, glm::value_ptr(lighting.globalAmbient));

    const int lightCount = static_cast<int>(lighting.lights.size());

    loc = glGetUniformLocation(programId, "uLighting.lightCount");
    assert(loc >= 0);
    glUniform1i(loc, lightCount);

    for (int i = 0; i < lightCount; i++) {
        const Light &light = lighting.lights[i];
        const std::string lightName = "uLighting.lights[" + std::to_string(i) + "]";

        loc = glGetUniformLocation(programId, (lightName + ".direction").c_str());
        assert(loc >= 0);
        glUniform3fv(loc, 1, glm::value_ptr(light.direction));

        loc = glGetUniformLocation(programId, (lightName + ".ambient").c_str());
        assert(loc >= 0);
        glUniform4fv(loc, 1, glm::value_ptr(light.ambient));

        loc = glGetUniformLocation(programId, (lightName + ".diffuse").c_str());
        assert(loc >= 0);
        glUniform4fv(loc, 1, glm::value_ptr(light.diffuse));
    }
}

void Renderer::renderMaterialChannel(const GLuint programId, const std::string &uniformPrefix, const GLint textureUnit, const MaterialChannel &channel) {
    char name[128] = {};
    GLint loc = 0;

    std::snprintf(name, sizeof(name), "%s.color", uniformPrefix.c_str());
    loc = glGetUniformLocation(programId, name);
    assert(loc >= 0);
    glUniform4fv(loc, 1, glm::value_ptr(channel.color));

    std::snprintf(name, sizeof(name), "%s.textureMapEnable", uniformPrefix.c_str());
    loc = glGetUniformLocation(programId, name);
    assert(loc >= 0);
    glUniform1f(loc, channel.textureMap ? 1.0f : 0.0f);

    if (channel.textureMap) {
        std::snprintf(name, sizeof(name), "%s.textureMap", uniformPrefix.c_str());
        loc = glGetUniformLocation(programId, name);
        assert(loc >= 0);
        glUniform1i(loc, textureUnit);
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, channel.textureMap);
    }
}

void Renderer::renderMaterial(const GLuint programId, const Material &material) {
    GLint textureUnit = -1;

    renderMaterialChannel(programId, "uMaterial.ambient", ++textureUnit, material.ambient);
    renderMaterialChannel(programId, "uMaterial.diffuse", ++textureUnit, material.diffuse);
    renderMaterialChannel(programId, "uMaterial.specular", ++textureUnit, material.specular);
    renderMaterialChannel(programId, "uMaterial.emissive", ++textureUnit, material.emissive);
}

void Renderer::renderModelTransform(const ShaderLocationMap &location, const float *transform) {
    assert(transform != nullptr);
    assert(location.uModel >= 0);
    glUniformMatrix4fv(location.uModel, 1, GL_FALSE, transform);
}

void Renderer::renderMesh(const Mesh &mesh) {
    assert(mesh.vao);

    // render the mesh
    glBindVertexArray(mesh.vao);
    if (mesh.indexed) {
        glDrawElements(mesh.primitiveType, static_cast<GLsizei>(mesh.count), mesh.indexDataType, nullptr);
    } else {
        glDrawArrays(mesh.primitiveType, 0, static_cast<GLsizei>(mesh.count));
    }
}

void Renderer::renderMeshes(const Mesh *meshes, const size_t count) {
    assert(meshes);

    for (size_t i = 0; i < count; i++) {
        renderMesh(meshes[i]);
    }
}

void Renderer::renderTranformation(const ShaderLocationMap &location, const Transformation &transformation) {
    auto model = transformation.computeMatrix();

    this->renderModelTransform(location, value_ptr(model));
}
