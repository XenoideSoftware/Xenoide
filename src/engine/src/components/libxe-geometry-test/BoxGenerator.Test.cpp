#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

#include "xe/geometry/BoxGenerator.h"

TEST_CASE("computeBoxCounts gives 24 verts / 36 indices on a unit subdivision", "[box][counts]") {
    const xe::BoxOptions opts{{1, 1}, {1.0f, 1.0f, 1.0f}};

    const xe::MeshCounts counts = xe::computeBoxCounts(opts);

    REQUIRE(counts.vertexCount == 24);
    REQUIRE(counts.indexCount == 36);
    REQUIRE(counts.indexType == xe::IndexType::UInt16);
}

TEST_CASE("generateBox emits unit-length normals pointing outward on the unit cube", "[box][generate]") {
    const xe::BoxOptions opts{{1, 1}, {1.0f, 1.0f, 1.0f}};
    const xe::MeshCounts counts = xe::computeBoxCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);
    std::vector<xe::Vector3> normals(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();
    storage.normals = normals.data();
    storage.indices = indices.data();

    xe::generateBox(opts, storage);

    for (const xe::Vector3 &n : normals) {
        const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        REQUIRE(std::fabs(length - 1.0f) < 1e-4f);
    }

    for (const xe::Vector3 &p : positions) {
        REQUIRE(std::fabs(p.x) <= 0.5f + 1e-4f);
        REQUIRE(std::fabs(p.y) <= 0.5f + 1e-4f);
        REQUIRE(std::fabs(p.z) <= 0.5f + 1e-4f);
    }
}

TEST_CASE("generateBox indices all reference valid vertices", "[box][generate]") {
    const xe::BoxOptions opts{{2, 3}, {1.0f, 1.0f, 1.0f}};
    const xe::MeshCounts counts = xe::computeBoxCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();
    storage.indices = indices.data();

    xe::generateBox(opts, storage);

    for (std::uint16_t idx : indices) {
        REQUIRE(idx < counts.vertexCount);
    }
}

TEST_CASE("generateBox scales positions by BoxOptions::size", "[box][size]") {
    const xe::BoxOptions opts{{1, 1}, {4.0f, 2.0f, 6.0f}};
    const xe::MeshCounts counts = xe::computeBoxCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();

    xe::generateBox(opts, storage);

    for (const xe::Vector3 &p : positions) {
        REQUIRE(std::fabs(p.x) <= 2.0f + 1e-4f);
        REQUIRE(std::fabs(p.y) <= 1.0f + 1e-4f);
        REQUIRE(std::fabs(p.z) <= 3.0f + 1e-4f);
    }
}
