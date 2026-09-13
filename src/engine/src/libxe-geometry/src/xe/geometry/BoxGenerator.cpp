
#include <xe/geometry/BoxGenerator.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <xe/math/Common.h>
#include <xe/math/Matrix.h>

namespace xe {
    namespace {
        constexpr int kFaceCount = 6;

        struct FaceRotation {
            //! Rotation angle in radians.
            float radians;
            //! Rotation axis (unit vector, right-handed).
            xe::Vector3 axis;
        };

        //! Six rotations that carry the base face (z = -0.5, normal = -Z) to each face of the cube.
        constexpr std::array<FaceRotation, kFaceCount> kFaceRotations = {{
            {0.0f, xe::Vector3{0.0f, 1.0f, 0.0f}},                  // -Z (front)
            {xe::pi<float> * 0.5f, {0.0f, 1.0f, 0.0f}},  // +X (right)
            {xe::pi<float>, {0.0f, 1.0f, 0.0f}},         // +Z (back)
            {xe::pi<float> * 1.5f, {0.0f, 1.0f, 0.0f}},  // -X (left)
            {xe::pi<float> * 0.5f, {1.0f, 0.0f, 0.0f}},  // -Y (bottom)
            {-xe::pi<float> * 0.5f, {1.0f, 0.0f, 0.0f}}, // +Y (top)
        }};

        template <typename T> T ratio(int a, int b) {
            return static_cast<T>(a) / static_cast<T>(b);
        }

        template <typename IndexT> void writeFaceIndices(IndexT *indices, std::size_t baseIndexOffset, std::uint32_t baseVertex, int slices, int stacks) {
            const int stride = slices + 1;

            std::size_t write = baseIndexOffset;
            for (int i = 0; i < slices; ++i) {
                for (int j = 0; j < stacks; ++j) {
                    const std::uint32_t p0 = baseVertex + (i + 0) + (j + 0) * stride;
                    const std::uint32_t p1 = baseVertex + (i + 1) + (j + 0) * stride;
                    const std::uint32_t p2 = baseVertex + (i + 0) + (j + 1) * stride;
                    const std::uint32_t p3 = baseVertex + (i + 1) + (j + 1) * stride;

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

    MeshCounts computeBoxCounts(const BoxOptions &opts) {
        assert(opts.division.x >= 1 && opts.division.y >= 1);

        const int slices = opts.division.x;
        const int stacks = opts.division.y;

        MeshCounts counts;
        counts.vertexCount = static_cast<std::size_t>(kFaceCount) * static_cast<std::size_t>(slices + 1) * static_cast<std::size_t>(stacks + 1);
        counts.indexCount = static_cast<std::size_t>(kFaceCount) * 6u * static_cast<std::size_t>(slices) * static_cast<std::size_t>(stacks);
        counts.indexType = pickIndexType(counts.vertexCount);
        return counts;
    }

    template <typename T> void generateBox(const BoxOptions &opts, const MeshStorage<T> &out) {
        assert(opts.division.x >= 1 && opts.division.y >= 1);

        const int slices = opts.division.x;
        const int stacks = opts.division.y;
        const std::size_t vertsPerFace = static_cast<std::size_t>(slices + 1) * static_cast<std::size_t>(stacks + 1);
        const std::size_t indicesPerFace = 6u * static_cast<std::size_t>(slices) * static_cast<std::size_t>(stacks);

        const xe::TVector<T, 3> size{static_cast<T>(opts.size.x), static_cast<T>(opts.size.y), static_cast<T>(opts.size.z)};

        const MeshCounts counts = computeBoxCounts(opts);

        for (int face = 0; face < kFaceCount; ++face) {
            const FaceRotation &rot = kFaceRotations[face];
            const tmat<T, 4, 4> rotation = xe::mat4Rotation(static_cast<T>(rot.radians), xe::TVector<T, 3>{static_cast<T>(rot.axis.x), static_cast<T>(rot.axis.y), static_cast<T>(rot.axis.z)});

            const std::size_t vertBase = static_cast<std::size_t>(face) * vertsPerFace;

            for (int i = 0; i < slices + 1; ++i) {
                const T ti = ratio<T>(i, slices);

                for (int j = 0; j < stacks + 1; ++j) {
                    const T tj = ratio<T>(j, stacks);
                    const std::size_t vertIndex = vertBase + static_cast<std::size_t>(i) + static_cast<std::size_t>(j) * static_cast<std::size_t>(slices + 1);

                    if (out.positions != nullptr) {
                        const xe::TVector<T, 4> basePos{lerp(T(-0.5), T(0.5), tj), lerp(T(0.5), T(-0.5), ti), T(-0.5), T(1)};
                        const xe::TVector<T, 4> rotated = rotation * basePos;
                        out.positions[vertIndex] = xe::TVector<T, 3>{rotated.x * size.x, rotated.y * size.y, rotated.z * size.z};
                    }

                    if (out.normals != nullptr) {
                        const xe::TVector<T, 4> baseNormal{T(0), T(0), T(-1), T(0)};
                        const xe::TVector<T, 4> rotated = rotation * baseNormal;
                        out.normals[vertIndex] = xe::TVector<T, 3>{rotated.x, rotated.y, rotated.z};
                    }

                    if (out.texCoords != nullptr) {
                        out.texCoords[vertIndex] = xe::TVector<T, 2>{lerp(T(0), T(1), tj), lerp(T(1), T(0), ti)};
                    }
                }
            }
        }

        if (out.indices != nullptr) {
            for (int face = 0; face < kFaceCount; ++face) {
                const std::size_t vertBase = static_cast<std::size_t>(face) * vertsPerFace;
                const std::size_t indexBase = static_cast<std::size_t>(face) * indicesPerFace;

                if (counts.indexType == IndexType::UInt16) {
                    writeFaceIndices(static_cast<std::uint16_t *>(out.indices), indexBase, static_cast<std::uint32_t>(vertBase), slices, stacks);
                } else {
                    writeFaceIndices(static_cast<std::uint32_t *>(out.indices), indexBase, static_cast<std::uint32_t>(vertBase), slices, stacks);
                }
            }
        }
    }

    template void generateBox<float>(const BoxOptions &, const MeshStorage<float> &);
    template void generateBox<double>(const BoxOptions &, const MeshStorage<double> &);
} // namespace xe
