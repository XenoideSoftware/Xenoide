
#include <glaze/gl.hpp>
#include <glaze/raii.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

static const char *VERT_SRC = R"glsl(
#version 330 core
layout(location = 0) in vec2 position; 
uniform float angle;
void main() {
    float c = cos(angle);
    float s = sin(angle);
    vec2 r = vec2(c * position.x - s * position.y,
                  s * position.x + c * position.y);
    gl_Position = vec4(r, 0.0, 1.0);
}
)glsl";

static const char *FRAG_SRC = R"glsl(
#version 330 core
out vec4 fragColor;
void main() {
    fragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)glsl";

static glaze::Unique<gl::Shader> compileShader(gl::ShaderType type, const char *src) {
    auto shader = glaze::makeUnique<gl::Shader>(type);

    gl::shaderSource(shader, 1, &src, nullptr);
    gl::compileShader(shader);

    if (!gl::getShaderiv(shader, gl::ShaderParameterName::eCompileStatus)) {
        std::cerr << "Shader compile error:\n" << gl::getShaderInfoLog(shader) << std::endl;
        std::exit(1);
    }
    return shader;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "glaze - spinning triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glazeLoadFunctions(glfwGetProcAddress);

    // Inner scope so every RAII-managed GL resource is destroyed while the
    // context is still current. Without it the Unique<> destructors fire
    // after glfwDestroyWindow/glfwTerminate, which tears down the GL context
    // and causes glDeleteProgram/glDeleteShader/... to raise
    // GL_INVALID_OPERATION (caught by GLAZE_GL_CHECK in debug builds).
    {
        // Geometry — equilateral triangle centred at origin
        static const float verts[] = {
            0.0f,
            0.5f,
            -0.433f,
            -0.25f,
            0.433f,
            -0.25f,
        };

        auto vao = glaze::makeUnique<gl::VertexArray>();
        gl::bindVertexArray(vao);

        auto vbo = glaze::makeUnique<gl::BufferId>();

        gl::bindBuffer(gl::BufferTarget::eArrayBuffer, vbo);
        gl::bufferData(gl::BufferTarget::eArrayBuffer, sizeof(verts), verts, gl::BufferUsage::eStaticDraw);
        gl::vertexAttribPointer(gl::AttribLocation(0), 2, gl::VertexAttribPointerType::eFloat, GL_FALSE, 0, nullptr);
        gl::enableVertexAttribArray(gl::AttribLocation(0));

        // Shader program
        auto prog = glaze::makeUnique<gl::Program>();

        {
            glaze::Unique<gl::Shader> vert = compileShader(gl::ShaderType::eVertexShader, VERT_SRC);
            glaze::Unique<gl::Shader> frag = compileShader(gl::ShaderType::eFragmentShader, FRAG_SRC);

            gl::attachShader(prog, vert);
            gl::attachShader(prog, frag);
        }

        gl::linkProgram(prog);

        if (!gl::getProgramiv(prog, gl::ProgramProperty::eLinkStatus)) {
            std::cerr << "Link error:\n" << gl::getProgramInfoLog(prog) << std::endl;
            return -1;
        }

        gl::UniformLocation const angleLoc = gl::getUniformLocation(prog, "angle");

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            int w, h;
            glfwGetFramebufferSize(window, &w, &h);

            gl::viewport(0, 0, w, h);

            gl::clearColor(0.15f, 0.15f, 0.2f, 1.0f);
            gl::clear(gl::ClearBufferMask::eColorBufferBit);

            gl::useProgram(prog);
            gl::uniform1f(angleLoc, static_cast<float>(glfwGetTime()));

            gl::bindVertexArray(vao);
            gl::drawArrays(gl::PrimitiveType::eTriangles, 0, 3);
            gl::bindVertexArray({});

            gl::useProgram({});

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
