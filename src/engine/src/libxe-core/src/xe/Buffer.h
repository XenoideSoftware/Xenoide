
#ifndef __XE_BUFFER_HPP__
#define __XE_BUFFER_HPP__

#include <cstddef>
#include <xe/Predef.h>

namespace xe {
    /**
     * @brief Memory area abstraction
     */
    class Buffer {
    public:
        virtual ~Buffer();

        /**
         * @brief Get the size of the buffer, in bytes.
         */
        virtual std::size_t getSize() const = 0;

        /**
         * @brief Read the buffer data into a user-defined memory region.
         */
        virtual void read(void *destination, const std::size_t size = 0, const std::size_t offset = 0, const std::size_t destinationOffset = 0) const = 0;

        /**
         * @brief Write a user-defined memory region to the buffer
         */
        virtual void write(const void *source, const std::size_t = 0, const std::size_t offset = 0, const std::size_t sourceOffset = 0) = 0;
    };
} // namespace xe

#endif
