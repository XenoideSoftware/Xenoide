
#ifndef __XE_GEOMETRY_BOXGENERATOR_H__
#define __XE_GEOMETRY_BOXGENERATOR_H__

#include <xe/geometry/MeshBuffers.h>
#include <xe/math/Math.h>

namespace xe {
    /**
     * @brief LOD + sizing options for a procedural box mesh.
     *
     * The box is centered on the origin and spans [-size/2, +size/2] along each axis.
     * Each of the 6 faces is tessellated as a division.x × division.y grid of quads.
     * Keeping both division components >= 1 is a precondition of computeBoxCounts /
     * generateBox; debug builds assert.
     */
    struct BoxOptions {
        //! Per-face grid resolution (slices × stacks). Each component must be >= 1.
        Vector2i division = {1, 1};

        //! Full extent of the box along each axis.
        Vector3 size = {1.0f, 1.0f, 1.0f};
    };

    /**
     * @brief Pre-compute storage sizing for a given BoxOptions.
     *
     * Callers use the returned MeshCounts to allocate per-stream storage before calling
     * generateBox. Pure function: same opts in, same counts out, no side effects.
     *
     * @param opts box parameters; division components must be >= 1
     * @return vertex / index counts and the chosen IndexType (UInt16 when vertexCount fits)
     */
    MeshCounts computeBoxCounts(const BoxOptions &opts);

    /**
     * @brief Fill caller-owned storage with box vertex and index data.
     *
     * Writes positions, normals, texCoords, and indices into the pointers held by `out`.
     * Any attribute pointer that is nullptr is skipped; the corresponding stream is not
     * written. The function allocates nothing. `out.indices`, when non-null, is interpreted
     * as uint16_t* or uint32_t* matching computeBoxCounts(opts).indexType.
     *
     * @tparam T floating-point element type (float or double). Only these are instantiated.
     * @param opts box parameters; must match the opts passed to computeBoxCounts
     * @param out pointer bundle into caller-owned storage
     */
    template <typename T> void generateBox(const BoxOptions &opts, const MeshStorage<T> &out);

    extern template void generateBox<float>(const BoxOptions &, const MeshStorage<float> &);
    extern template void generateBox<double>(const BoxOptions &, const MeshStorage<double> &);
} // namespace xe

#endif
