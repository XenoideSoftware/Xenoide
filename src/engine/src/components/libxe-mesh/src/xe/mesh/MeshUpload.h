
#ifndef __XE_MESH_MESHUPLOAD_H__
#define __XE_MESH_MESHUPLOAD_H__

#include <array>
#include <cstdint>
#include <vector>

#include <tl/expected.hpp>

#include <xe/geometry/MeshBuffers.h>
#include <xe/graphics/BufferDescriptor.h>
#include <xe/graphics/RenderBackend.h>
#include <xe/graphics/types.h>

namespace xe {
    //! Bitmask identifying which attribute streams an uploadMesh caller wants on the GPU.
    enum MeshAttribFlags : std::uint32_t {
        MeshAttribPosition = 1u << 0,
        MeshAttribNormal = 1u << 1,
        MeshAttribTexCoord0 = 1u << 2,
    };

    /**
     * @brief Caller-side knobs for uploadMesh.
     *
     * attribMask selects which streams are uploaded. Each bit set in `attribMask` consumes the
     * matching pointer from `MeshStorage<float>` (so a caller that requests MeshAttribNormal
     * must supply a non-null `storage.normals`). `locations[k]` gives the shader attribute
     * location assigned to the k-th entry of { Position, Normal, TexCoord0 } whenever the
     * matching bit is set.
     */
    struct MeshUploadOptions {
        //! Bit OR of MeshAttribFlags values. Defaults to all three enabled.
        std::uint32_t attribMask = MeshAttribPosition | MeshAttribNormal | MeshAttribTexCoord0;

        //! Shader attribute locations for Position, Normal, TexCoord0 in that fixed order.
        std::array<int, 3> locations = {0, 1, 2};

        //! Buffer usage hint applied to every attribute and index buffer created by uploadMesh.
        BufferUsage usage = BufferUsage::StaticDraw;
    };

    /**
     * @brief Aggregate of the backend resources created by uploadMesh.
     *
     * Ownership semantics: the caller is responsible for calling destroyMeshGeometry (or manually
     * invoking the matching destroy entry points on the vtable) to release `geometry`, `layout`,
     * every element of `attributeBuffers`, and `indexBuffer` when the mesh is no longer needed.
     */
    struct MeshGeometry {
        //! Bound VAO handle, ready for draw calls.
        GeometryHandle geometry;

        //! Vertex layout used to build `geometry`.
        VertexLayoutHandle layout;

        //! Per-attribute vertex buffer handles, in the same order as MeshUploadOptions::locations.
        std::vector<BufferHandle> attributeBuffers;

        //! Index buffer handle; zero-initialized / invalid when the mesh has no indices.
        BufferHandle indexBuffer;
    };

    /**
     * @brief Upload a CPU mesh into GPU resources wrapped by an xe::Geometry.
     *
     * Creates one backend buffer per enabled attribute stream plus one index buffer, a matching
     * VertexLayout in Explicit resolve mode, and a Geometry that binds them. On any failure the
     * function rolls back every resource it already created before returning the error.
     *
     * Storage is interpreted per `counts`: `storage.positions` / `normals` / `texCoords` must
     * each cover `counts.vertexCount` elements (when the matching bit in opts.attribMask is set),
     * and `storage.indices` is reinterpreted as `uint16_t *` or `uint32_t *` per `counts.indexType`.
     *
     * @param vtable render backend dispatch table; every entry point touched by this function must be populated
     * @param ctx backend context bound to the caller's current GL/GPU context
     * @param counts sizing metadata returned by computeXCounts(opts) on the producing generator
     * @param storage pointers to caller-owned CPU-side data; unused streams may be nullptr
     * @param opts upload knobs (default: all three attributes, locations {0,1,2}, StaticDraw)
     * @return populated MeshGeometry on success; a BackendError on the first failure (resources rolled back)
     */
    tl::expected<MeshGeometry, BackendError> uploadMesh(
        const RenderDeviceBackendVTable &vtable, RenderDeviceBackendContext *ctx, const MeshCounts &counts, const MeshStorage<float> &storage, const MeshUploadOptions &opts = {}
    );

    /**
     * @brief Destroy every handle owned by a MeshGeometry and zero it out.
     *
     * Safe to call on a partially populated MeshGeometry (e.g. one built manually); only handles
     * whose `isValid()` returns true are forwarded to the backend destroy entry points.
     *
     * @param vtable render backend dispatch table; destroyBuffer / destroyVertexLayout / destroyGeometry must be populated
     * @param ctx backend context the resources were created under
     * @param mesh mesh whose handles will be released and reset to the default (invalid) value
     */
    void destroyMeshGeometry(const RenderDeviceBackendVTable &vtable, RenderDeviceBackendContext *ctx, MeshGeometry &mesh);
} // namespace xe

#endif
