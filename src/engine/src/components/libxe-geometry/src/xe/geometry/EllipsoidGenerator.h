
#ifndef __XE_GEOMETRY_ELLIPSOIDGENERATOR_H__
#define __XE_GEOMETRY_ELLIPSOIDGENERATOR_H__

#include <xe/geometry/MeshBuffers.h>
#include <xe/math/Math.h>

namespace xe {
    /**
     * @brief LOD + sizing options for a procedural ellipsoid mesh (UV-sphere parameterisation).
     *
     * The ellipsoid is centered on the origin with per-axis radii `radii`. Longitude is
     * sampled in `slices` divisions (0..2π); latitude in `stacks` divisions (0..π), so
     * `stacks` must be >= 2 to produce a closed surface. Debug builds assert the preconditions.
     */
    struct EllipsoidOptions {
        //! Longitude subdivisions around the equator. Must be >= 3.
        int slices = 16;

        //! Latitude subdivisions from pole to pole. Must be >= 2.
        int stacks = 16;

        //! Per-axis radii. The unit sphere is radii == {1,1,1}.
        Vector3 radii = {1.0f, 1.0f, 1.0f};
    };

    /**
     * @brief Pre-compute storage sizing for a given EllipsoidOptions.
     * @param opts ellipsoid parameters; slices >= 3, stacks >= 2
     * @return vertex / index counts and the chosen IndexType
     */
    MeshCounts computeEllipsoidCounts(const EllipsoidOptions &opts);

    /**
     * @brief Fill caller-owned storage with ellipsoid vertex and index data.
     *
     * Writes positions, normals, texCoords, and indices into the pointers held by `out`.
     * Null attribute pointers are skipped silently. Normals are unit length even when
     * radii is non-uniform (computed as normalize((x/rx^2, y/ry^2, z/rz^2))).
     *
     * @tparam T floating-point element type (float or double). Only these are instantiated.
     * @param opts ellipsoid parameters; must match those passed to computeEllipsoidCounts
     * @param out pointer bundle into caller-owned storage
     */
    template <typename T> void generateEllipsoid(const EllipsoidOptions &opts, const MeshStorage<T> &out);

    extern template void generateEllipsoid<float>(const EllipsoidOptions &, const MeshStorage<float> &);
    extern template void generateEllipsoid<double>(const EllipsoidOptions &, const MeshStorage<double> &);
} // namespace xe

#endif
