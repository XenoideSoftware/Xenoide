
#include <xe/geometry/EllipsoidGenerator.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <xe/math/Common.h>
#include <xe/math/Vector.h>

namespace xe {
    namespace {
        template <typename T> T ratio(int a, int b) {
            return static_cast<T>(a) / static_cast<T>(b);
        }

        template <typename IndexT> void writeEllipsoidIndices(IndexT *indices, int slices, int stacks) {
            const int stride = slices + 1;

            std::size_t write = 0;
            for (int i = 0; i < stacks; ++i) {
                for (int j = 0; j < slices; ++j) {
                    const std::uint32_t p0 = (i + 0) * stride + (j + 0);
                    const std::uint32_t p1 = (i + 0) * stride + (j + 1);
                    const std::uint32_t p2 = (i + 1) * stride + (j + 0);
                    const std::uint32_t p3 = (i + 1) * stride + (j + 1);

                    indices[write++] = static_cast<IndexT>(p0);
                    indices[write++] = static_cast<IndexT>(p2);
                    indices[write++] = static_cast<IndexT>(p1);

                    indices[write++] = static_cast<IndexT>(p1);
                    indices[write++] = static_cast<IndexT>(p2);
                    indices[write++] = static_cast<IndexT>(p3);
                }
            }
        }
    } // namespace

    MeshCounts computeEllipsoidCounts(const EllipsoidOptions &opts) {
        assert(opts.slices >= 3);
        assert(opts.stacks >= 2);

        MeshCounts counts;
        counts.vertexCount = static_cast<std::size_t>(opts.slices + 1) * static_cast<std::size_t>(opts.stacks + 1);
        counts.indexCount = 6u * static_cast<std::size_t>(opts.slices) * static_cast<std::size_t>(opts.stacks);
        counts.indexType = pickIndexType(counts.vertexCount);
        return counts;
    }

    template <typename T> void generateEllipsoid(const EllipsoidOptions &opts, const MeshStorage<T> &out) {
        assert(opts.slices >= 3);
        assert(opts.stacks >= 2);

        const int slices = opts.slices;
        const int stacks = opts.stacks;
        const xe::TVector<T, 3> radii{static_cast<T>(opts.radii.x), static_cast<T>(opts.radii.y), static_cast<T>(opts.radii.z)};
        const xe::TVector<T, 3> invSquaredRadii{T(1) / (radii.x * radii.x), T(1) / (radii.y * radii.y), T(1) / (radii.z * radii.z)};

        const MeshCounts counts = computeEllipsoidCounts(opts);

        const T twoPi = T(2) * pi<T>;

        for (int i = 0; i < stacks + 1; ++i) {
            const T tv = ratio<T>(i, stacks);
            const T phi = tv * pi<T>;
            const T sinPhi = std::sin(phi);
            const T cosPhi = std::cos(phi);

            for (int j = 0; j < slices + 1; ++j) {
                const T tu = ratio<T>(j, slices);
                const T theta = tu * twoPi;
                const T sinTheta = std::sin(theta);
                const T cosTheta = std::cos(theta);

                const std::size_t vertIndex = static_cast<std::size_t>(i) * static_cast<std::size_t>(slices + 1) + static_cast<std::size_t>(j);

                const xe::TVector<T, 3> position{radii.x * sinPhi * cosTheta, radii.y * cosPhi, radii.z * sinPhi * sinTheta};

                if (out.positions != nullptr) {
                    out.positions[vertIndex] = position;
                }

                if (out.normals != nullptr) {
                    const xe::TVector<T, 3> n{position.x * invSquaredRadii.x, position.y * invSquaredRadii.y, position.z * invSquaredRadii.z};
                    out.normals[vertIndex] = normalize(n);
                }

                if (out.texCoords != nullptr) {
                    out.texCoords[vertIndex] = xe::TVector<T, 2>{tu, T(1) - tv};
                }
            }
        }

        if (out.indices != nullptr) {
            if (counts.indexType == IndexType::UInt16) {
                writeEllipsoidIndices(static_cast<std::uint16_t *>(out.indices), slices, stacks);
            } else {
                writeEllipsoidIndices(static_cast<std::uint32_t *>(out.indices), slices, stacks);
            }
        }
    }

    template void generateEllipsoid<float>(const EllipsoidOptions &, const MeshStorage<float> &);
    template void generateEllipsoid<double>(const EllipsoidOptions &, const MeshStorage<double> &);
} // namespace xe
