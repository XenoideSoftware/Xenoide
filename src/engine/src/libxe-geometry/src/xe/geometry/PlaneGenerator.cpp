
#include <xe/geometry/PlaneGenerator.h>

#include <cassert>
#include <cstdint>
#include <xe/math/Common.h>

namespace xe {
    namespace {
        template <typename T> T ratio(int a, int b) {
            return static_cast<T>(a) / static_cast<T>(b);
        }

        template <typename IndexT> void writePlaneIndices(IndexT *indices, int slices, int stacks) {
            const int stride = slices + 1;

            std::size_t write = 0;
            for (int i = 0; i < slices; ++i) {
                for (int j = 0; j < stacks; ++j) {
                    const std::uint32_t p0 = (i + 0) + (j + 0) * stride;
                    const std::uint32_t p1 = (i + 1) + (j + 0) * stride;
                    const std::uint32_t p2 = (i + 0) + (j + 1) * stride;
                    const std::uint32_t p3 = (i + 1) + (j + 1) * stride;

                    indices[write++] = static_cast<IndexT>(p0);
                    indices[write++] = static_cast<IndexT>(p1);
                    indices[write++] = static_cast<IndexT>(p2);

                    indices[write++] = static_cast<IndexT>(p1);
                    indices[write++] = static_cast<IndexT>(p3);
                    indices[write++] = static_cast<IndexT>(p2);
                }
            }
        }
    } // namespace

    MeshCounts computePlaneCounts(const PlaneOptions &opts) {
        assert(opts.division.x >= 1 && opts.division.y >= 1);

        MeshCounts counts;
        counts.vertexCount = static_cast<std::size_t>(opts.division.x + 1) * static_cast<std::size_t>(opts.division.y + 1);
        counts.indexCount = 6u * static_cast<std::size_t>(opts.division.x) * static_cast<std::size_t>(opts.division.y);
        counts.indexType = pickIndexType(counts.vertexCount);
        return counts;
    }

    template <typename T> void generatePlane(const PlaneOptions &opts, const MeshStorage<T> &out) {
        assert(opts.division.x >= 1 && opts.division.y >= 1);

        const int slices = opts.division.x;
        const int stacks = opts.division.y;
        const T halfWidth = static_cast<T>(opts.size.x) * T(0.5);
        const T halfHeight = static_cast<T>(opts.size.y) * T(0.5);

        const MeshCounts counts = computePlaneCounts(opts);

        for (int i = 0; i < slices + 1; ++i) {
            const T ti = ratio<T>(i, slices);

            for (int j = 0; j < stacks + 1; ++j) {
                const T tj = ratio<T>(j, stacks);
                const std::size_t vertIndex = static_cast<std::size_t>(i) + static_cast<std::size_t>(j) * static_cast<std::size_t>(slices + 1);

                if (out.positions != nullptr) {
                    out.positions[vertIndex] = xe::TVector<T, 3>{lerp(-halfHeight, halfHeight, tj), lerp(halfWidth, -halfWidth, ti), T(0)};
                }

                if (out.normals != nullptr) {
                    out.normals[vertIndex] = xe::TVector<T, 3>{T(0), T(0), T(-1)};
                }

                if (out.texCoords != nullptr) {
                    out.texCoords[vertIndex] = xe::TVector<T, 2>{lerp(T(0), T(1), tj), lerp(T(1), T(0), ti)};
                }
            }
        }

        if (out.indices != nullptr) {
            if (counts.indexType == IndexType::UInt16) {
                writePlaneIndices(static_cast<std::uint16_t *>(out.indices), slices, stacks);
            } else {
                writePlaneIndices(static_cast<std::uint32_t *>(out.indices), slices, stacks);
            }
        }
    }

    template void generatePlane<float>(const PlaneOptions &, const MeshStorage<float> &);
    template void generatePlane<double>(const PlaneOptions &, const MeshStorage<double> &);
} // namespace xe
