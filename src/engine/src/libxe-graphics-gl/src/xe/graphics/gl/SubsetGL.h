
#ifndef __XE_GRAPHICS_GL_SUBSETGL_HPP__
#define __XE_GRAPHICS_GL_SUBSETGL_HPP__

#include <xe/graphics/Subset.h>

#include "gl.h"

namespace xe {
    class SubsetGL : public Subset {
    public:
        explicit SubsetGL(const SubsetDescriptor &desc);

        virtual ~SubsetGL();

        virtual int getBufferCount() const override;

    public:
        GLuint getID() const {
            return id;
        }

    private:
        GLuint id = 0;
        SubsetDescriptor descriptor;
    };
} // namespace xe

#endif
