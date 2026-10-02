
#ifndef __XE_GRAPHICS_SHADER_HPP__
#define __XE_GRAPHICS_SHADER_HPP__

#include <string>

namespace xe {
    enum class ShaderType {
        Vertex,
        Fragment,
        Geometry,
    };

    class Shader {
    public:
        virtual ~Shader();

        virtual ShaderType getType() const = 0;

        virtual std::string getSource() const = 0;
    };
} // namespace xe

#endif
