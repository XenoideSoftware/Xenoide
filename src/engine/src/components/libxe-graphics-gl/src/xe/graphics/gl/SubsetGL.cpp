
#include "SubsetGL.h"
#include "Conversion.h"

#include <cassert>

namespace xe {
    SubsetGL::SubsetGL(const SubsetDescriptor &desc) : descriptor(desc) {
        // setup the subset arrays based on the vertex format and the mapping information
        /*
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        for (std::size_t i = 0; i < desc.attribs.size(); i++) {
            const SubsetVertexAttrib &attrib = desc.attribs[i];
            const BufferGL *buffer = buffers[attrib.bufferIndex];

            glBindBuffer(buffer->getTarget(), buffer->getID());
            glEnableVertexAttribArray(attrib.shaderLocation);

            glVertexAttribPointer(
                attrib.shaderLocation,
                attrib.size,
                convertToGL(attrib.type),
                convertToGL(attrib.normalized),
                static_cast<GLsizei>(attrib.stride),
                reinterpret_cast<const void *>(attrib.bufferOffset)
            );
        }

                indexBuffer = dynamic_cast<const BufferGL*>(desc.indexBuffer);

        if (indexBuffer) {
            glBindBuffer(indexBuffer->getTarget(), indexBuffer->getID());
        }

        glBindVertexArray(0);
        */
    }

    SubsetGL::~SubsetGL() {
        if (id) {
            glDeleteVertexArrays(1, &id);
        }
    }
} // namespace xe
