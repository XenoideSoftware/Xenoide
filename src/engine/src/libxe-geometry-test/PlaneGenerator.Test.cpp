#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

#include "xe/geometry/PlaneGenerator.h"

TEST_CASE("computePlaneCounts picks a quad for the unit subdivision", "[plane][counts]") {
    const xe::PlaneOptions opts{{1, 1}, {1.0f, 1.0f}};

    const xe::MeshCounts counts = xe::computePlaneCounts(opts);

    REQUIRE(counts.vertexCount == 4);
    REQUIRE(counts.indexCount == 6);
    REQUIRE(counts.indexType == xe::IndexType::UInt16);
}

TEST_CASE("computePlaneCounts crosses into UInt32 when division pushes vertex count past 65535", "[plane][counts]") {
    const xe::PlaneOptions opts{{300, 300}, {1.0f, 1.0f}}; // (301 * 301) = 90601 verts.

    const xe::MeshCounts counts = xe::computePlaneCounts(opts);

    REQUIRE(counts.vertexCount == 90601);
    REQUIRE(counts.indexType == xe::IndexType::UInt32);
}

TEST_CASE("generatePlane skips nullptr streams and leaves surrounding memory untouched", "[plane][generate]") {
    const xe::PlaneOptions opts{{2, 2}, {1.0f, 1.0f}};
    const xe::MeshCounts counts = xe::computePlaneCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    // Sentinel buffer that the generator must not touch: positions-sized, but we pass nullptr for normals.
    constexpr float sentinel = 1234.5f;
    std::vector<xe::Vector3> normalsCanary(counts.vertexCount, xe::Vector3{sentinel, sentinel, sentinel});

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();
    storage.normals = nullptr;
    storage.texCoords = nullptr;
    storage.indices = indices.data();

    xe::generatePlane(opts, storage);

    for (const xe::Vector3 &n : normalsCanary) {
        REQUIRE(n.x == sentinel);
        REQUIRE(n.y == sentinel);
        REQUIRE(n.z == sentinel);
    }

    for (std::uint16_t idx : indices) {
        REQUIRE(idx < counts.vertexCount);
    }
}

TEST_CASE("generatePlane yields a single quad on the unit subdivision", "[plane][generate]") {
    const xe::PlaneOptions opts{{1, 1}, {2.0f, 2.0f}};
    const xe::MeshCounts counts = xe::computePlaneCounts(opts);

    std::vector<xe::Vector3> positions(counts.vertexCount);
    std::vector<xe::Vector3> normals(counts.vertexCount);
    std::vector<xe::Vector2> texCoords(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    xe::MeshStorage<float> storage;
    storage.positions = positions.data();
    storage.normals = normals.data();
    storage.texCoords = texCoords.data();
    storage.indices = indices.data();

    xe::generatePlane(opts, storage);

    for (const xe::Vector3 &n : normals) {
        REQUIRE(n.x == 0.0f);
        REQUIRE(n.y == 0.0f);
        REQUIRE(n.z == -1.0f);
    }

    for (const xe::Vector3 &p : positions) {
        REQUIRE(p.z == 0.0f);
        REQUIRE(p.x >= -1.0f);
        REQUIRE(p.x <= 1.0f);
        REQUIRE(p.y >= -1.0f);
        REQUIRE(p.y <= 1.0f);
    }

    for (std::uint16_t idx : indices) {
        REQUIRE(idx < counts.vertexCount);
    }
}

TEST_CASE("generatePlane<double> compiles and produces double-precision output", "[plane][double]") {
    const xe::PlaneOptions opts{{1, 1}, {1.0f, 1.0f}};
    const xe::MeshCounts counts = xe::computePlaneCounts(opts);

    std::vector<xe::TVector<double, 3>> positions(counts.vertexCount);
    std::vector<std::uint16_t> indices(counts.indexCount);

    xe::MeshStorage<double> storage;
    storage.positions = positions.data();
    storage.indices = indices.data();

    xe::generatePlane<double>(opts, storage);

    REQUIRE(positions[0].z == 0.0);
}
