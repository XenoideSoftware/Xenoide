#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

#include "xe/geometry/EllipsoidGenerator.h"

TEST_CASE("computeEllipsoidCounts scales with slices and stacks", "[ellipsoid][counts]") {
    const xe::EllipsoidOptions opts{16, 16, {1.0f, 1.0f, 1.0f}};

    const xe::MeshCounts counts = xe::computeEllipsoidCounts(opts);

    REQUIRE(counts.vertexCount == (16u + 1u) * (16u + 1u));
    REQUIRE(counts.indexCount == 6u * 16u * 16u);
    REQUIRE(counts.indexType == xe::IndexType::UInt16);
}

TEST_CASE("generateEllipsoid places unit-sphere vertices on the unit sphere", "[ellipsoid][generate]") {
    const xe::EllipsoidOptions opts{16, 16, {1.0f, 1.0f, 1.0f}};
    const xe::MeshCounts counts = xe::computeEllipsoidCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);
    std::vector<xe::Vector3> normals(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();
    storage.normals = normals.data();
    storage.indices = indices.data();

    xe::generateEllipsoid(opts, storage);

    for (const xe::Vector3 &p : positions) {
        const float r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        REQUIRE(std::fabs(r - 1.0f) < 1e-4f);
    }

    for (const xe::Vector3 &n : normals) {
        const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(std::fabs(length - 1.0f) < 1e-4f);
    }

    for (std::uint16_t idx : indices) {
        REQUIRE(idx < counts.vertexCount);
    }
}

TEST_CASE("generateEllipsoid normals remain unit length on a non-uniform ellipsoid", "[ellipsoid][normals]") {
    const xe::EllipsoidOptions opts{12, 8, {3.0f, 1.0f, 2.0f}};
    const xe::MeshCounts counts = xe::computeEllipsoidCounts(opts);

    std::vector<xe::Vector3> normals(counts.vertexCount);

    xe::MeshStorage<float> storage;
    storage.normals = normals.data();

    xe::generateEllipsoid(opts, storage);

    for (const xe::Vector3 &n : normals) {
        const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(std::fabs(length - 1.0f) < 1e-4f);
    }
}

TEST_CASE("generateEllipsoid crosses over to UInt32 indices when very tessellated", "[ellipsoid][counts]") {
    const xe::EllipsoidOptions opts{400, 400, {1.0f, 1.0f, 1.0f}}; // 401*401 = 160801 vertices.

    const xe::MeshCounts counts = xe::computeEllipsoidCounts(opts);

    REQUIRE(counts.vertexCount > 65535);
    REQUIRE(counts.indexType == xe::IndexType::UInt32);
}
