
#ifndef __XE_GEOMETRY_PLANEGENERATOR_H__
#define __XE_GEOMETRY_PLANEGENERATOR_H__

#include <xe/geometry/MeshBuffers.h>
#include <xe/math/Math.h>

namespace xe {
    /**
     * @brief LOD + sizing options for a procedural plane mesh.
     *
     * The plane lies on XY (normal = -Z), centered on the origin, covering
     * [-size/2, +size/2] in X and Y. Division components control slices × stacks
     * and must each be >= 1 (debug asserts).
     */
    struct PlaneOptions {
        //! Grid resolution (slices × stacks). Each component must be >= 1.
        Vector2i division = {1, 1};

        //! Full extent of the plane along X and Y.
        Vector2 size = {1.0f, 1.0f};
    };

    /**
     * @brief Pre-compute storage sizing for a given PlaneOptions.
     * @param opts plane parameters; division components must be >= 1
     * @return vertex / index counts and the chosen IndexType
     */
    MeshCounts computePlaneCounts(const PlaneOptions &opts);

    /**
     * @brief Fill caller-owned storage with plane vertex and index data.
     *
     * Writes positions, normals (all {0, 0, -1}), texCoords, and indices into the
     * pointers held by `out`. Null attribute pointers are skipped silently.
     *
     * @tparam T floating-point element type (float or double). Only these are instantiated.
     * @param opts plane parameters; must match those passed to computePlaneCounts
     * @param out pointer bundle into caller-owned storage
     */
    template <typename T> void generatePlane(const PlaneOptions &opts, const MeshStorage<T> &out);

    extern template void generatePlane<float>(const PlaneOptions &, const MeshStorage<float> &);
    extern template void generatePlane<double>(const PlaneOptions &, const MeshStorage<double> &);
} // namespace xe

#endif
