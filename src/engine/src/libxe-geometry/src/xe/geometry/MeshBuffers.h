
#ifndef __XE_GEOMETRY_MESHBUFFERS_H__
#define __XE_GEOMETRY_MESHBUFFERS_H__

#include <cstddef>
#include <cstdint>
#include <xe/math/Math.h>

namespace xe {
    //! Index storage width used by a generated mesh.
    enum class IndexType { UInt16, UInt32 };

    /**
     * @brief Sizing metadata returned by each computeXCounts(opts) function.
     *
     * Callers size their own storage from this (vertexCount * sizeof(attribute) bytes per
     * attribute stream, indexCount * indexByteSize(indexType) bytes for indices) and then
     * invoke the matching generateX(opts, storage) call. Keeping the two phases separate
     * is what lets libxe-geometry stay allocation-free.
     */
    struct MeshCounts {
        //! Number of vertices each non-null attribute stream must hold.
        std::size_t vertexCount = 0;

        //! Number of indices the index stream must hold.
        std::size_t indexCount = 0;

        //! Width picked for indices (UInt16 when vertexCount <= 65535, else UInt32).
        IndexType indexType = IndexType::UInt16;
    };

    /**
     * @brief Non-owning pointer bundle passed to generateX(opts, storage).
     *
     * Every pointer field is optional. A nullptr in an attribute field tells the generator
     * to skip that stream (generators write nothing there and touch no adjacent memory).
     * The indices pointer, when non-null, is reinterpreted per the companion MeshCounts::indexType.
     *
     * @tparam T floating-point component type for positions/normals/texcoords (float or double).
     */
    template <typename T> struct MeshStorage {
        //! At least MeshCounts::vertexCount xe::TVector<T,3> slots, or nullptr to skip.
        xe::TVector<T, 3> *positions = nullptr;

        //! At least MeshCounts::vertexCount xe::TVector<T,3> slots, or nullptr to skip.
        xe::TVector<T, 3> *normals = nullptr;

        //! At least MeshCounts::vertexCount xe::TVector<T,2> slots, or nullptr to skip.
        xe::TVector<T, 2> *texCoords = nullptr;

        //! Reinterpreted as uint16_t* or uint32_t* per MeshCounts::indexType. Required when indexCount > 0.
        void *indices = nullptr;
    };

    /**
     * @brief Byte size of a single index for the given width.
     * @param type index width picked by the generator
     * @return 2 for UInt16, 4 for UInt32
     */
    constexpr std::size_t indexByteSize(IndexType type) {
        return type == IndexType::UInt16 ? sizeof(std::uint16_t) : sizeof(std::uint32_t);
    }

    /**
     * @brief Pick the narrowest index width that fits a given vertex count.
     * @param vertexCount number of vertices the mesh will reference
     * @return UInt16 when vertexCount <= 65535, UInt32 otherwise
     */
    constexpr IndexType pickIndexType(std::size_t vertexCount) {
        return vertexCount <= 0xFFFFu ? IndexType::UInt16 : IndexType::UInt32;
    }
} // namespace xe

#endif
