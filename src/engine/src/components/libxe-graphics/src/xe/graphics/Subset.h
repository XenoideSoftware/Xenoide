
#ifndef __XE_GRAPHICS_SUBSET_HPP__
#define __XE_GRAPHICS_SUBSET_HPP__

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <xe/DataType.h>
#include <xe/Predef.h>

namespace xe {
    class Buffer;
}

namespace xe {
    /**
     * @brief Interpretation information for a given subset. Used in rendering operations.
     */
    struct SubsetEnvelope {
        //! From wich vertex we will render
        int vertexStart = 0;

        //! How many vertices we must use for render
        int vertexCount = 0;
    };

    /**
     * @brief Describes a Vertex Attribute for use in the vertex shader
     */
    struct VertexAttribute {
        //! attribute Name in the Vertex Shader
        std::string name;

        //! Basic data type
        DataType type = DataType::Unknown;

        //! Attribute dimension (1, 2, 3 or 4)
        int size = 0;
    };

    /**
     * @brief Describes a Vertex Attribute for use in the vertex shader
     */
    struct SubsetVertexAttrib {
        //! attribute layout location in the vertex shader
        int shaderLocation = 0;

        //! Basic data type
        DataType type = DataType::Unknown;

        //! attribute dimension (1, 2, 3 or 4)
        int size = 0;

        bool normalized = false;

        std::size_t stride = 0;

        //!
        std::size_t bufferIndex = 0;

        //!
        std::size_t bufferOffset = 0;
    };

    /**
     * @brief Describes a geometry subset.
     */
    struct SubsetDescriptor {
        std::vector<Buffer *> buffers;
        std::vector<SubsetVertexAttrib> attribs;
        Buffer *indexBuffer = nullptr;
    };

    /**
     * @brief Geometry Subset
     *
     * This class abstracts a (possibly indexed) Geometry Subset, independent of the Vertex Layout.
     * The Vertex Layout just specifies the different attributes supported by each vertex stored in one or many
     * buffer(s).
     */
    class Subset {
    public:
        virtual ~Subset();

        /**
         * @brief Get the count of buffers attached to the geometric subset.
         */
        virtual int getBufferCount() const = 0;

        /**
         * @brief Get a mutable pointer to a buffer allocated in GPU memory. This buffer stores geometric data
         */
        virtual Buffer *getBuffer(const int index) = 0;

        /**
         * @brief Get a mutable pointer to a buffer allocated in GPU memory. This buffer stores indexation data
         */
        virtual Buffer *getIndexBuffer() = 0;

        /**
         * @brief Get a constant pointer to a buffer allocated in GPU memory. This buffer stores geometric data
         */
        virtual const Buffer *getBuffer(const int index) const = 0;

        /**
         * @brief Get a constant pointer to a buffer allocated in GPU memory. This buffer stores indexation data
         */
        virtual const Buffer *getIndexBuffer() const = 0;
    };
} // namespace xe

#endif
