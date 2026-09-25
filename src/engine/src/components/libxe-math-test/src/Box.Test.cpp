
#include "xe/math/Box.h"
#include <catch2/catch_all.hpp>
#include <iostream>
#include <sstream>

TEST_CASE("Box default constructor zeroes edges", "[math][box]") {
    xe::Box box;

    REQUIRE(box.getMinEdge() == xe::Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(box.getMaxEdge() == xe::Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(box.isValid());
}

TEST_CASE("Box constructor from one vector initializes edges", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, 3.0f}};

    REQUIRE(box.getMinEdge() == xe::Vector3(1.0f, 2.0f, 3.0f));
    REQUIRE(box.getMaxEdge() == xe::Vector3(1.0f, 2.0f, 3.0f));
    REQUIRE(box.isValid());
}

TEST_CASE("Box constructor from two vectors initializes edges", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};

    REQUIRE(box.getMinEdge() == xe::Vector3(-1.0f, 2.0f, -3.0f));
    REQUIRE(box.getMaxEdge() == xe::Vector3(1.0f, 8.0f, 3.0f));
    REQUIRE(box.isValid());
}

TEST_CASE("Box getEdge should get a specific point from the box", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};

    box = {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};

    REQUIRE(box.getEdge(0) == xe::Vector3(-1.0f, -1.0f, -1.0f));
    REQUIRE(box.getEdge(1) == xe::Vector3(1.0f, -1.0f, -1.0f));

    REQUIRE(box.getEdge(2) == xe::Vector3(-1.0f, 1.0f, -1.0f));
    REQUIRE(box.getEdge(3) == xe::Vector3(1.0f, 1.0f, -1.0f));

    REQUIRE(box.getEdge(4) == xe::Vector3(-1.0f, -1.0f, 1.0f));
    REQUIRE(box.getEdge(5) == xe::Vector3(1.0f, -1.0f, 1.0f));

    REQUIRE(box.getEdge(6) == xe::Vector3(-1.0f, 1.0f, 1.0f));
    REQUIRE(box.getEdge(7) == xe::Vector3(1.0f, 1.0f, 1.0f));
}

TEST_CASE("Box getSize returns a vector with the computed size", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};
    REQUIRE(box.getSize() == xe::Vector3(2.0f, 6.0f, 6.0f));
}

TEST_CASE("Box getCenter returns a vector with average between min and max edges", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};
    REQUIRE(box.getCenter() == xe::Vector3(0.0f, 5.0f, 0.0f));
}

TEST_CASE("Box expand does not change the box when using a point inside of it", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};

    box.expand(box.getCenter());

    REQUIRE(box.getSize() == xe::Vector3(2.0f, 6.0f, 6.0f));
    REQUIRE(box.getCenter() == xe::Vector3(0.0f, 5.0f, 0.0f));
}

TEST_CASE("Box expand does not change the box when using a point-like box inside of it", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};

    box.expand(xe::Box{box.getCenter()});

    REQUIRE(box.getSize() == xe::Vector3(2.0f, 6.0f, 6.0f));
    REQUIRE(box.getCenter() == xe::Vector3(0.0f, 5.0f, 0.0f));
}

TEST_CASE("Box expand increases the box when using a point outside of it", "[math][box]") {
    xe::Box box{{1.0f, 2.0f, -3.0f}, {-1.0f, 8.0f, 3.0f}};

    const auto prevCenter = box.getCenter();
    const auto prevSize = box.getSize();

    box.expand(xe::Vector3(-10.0f, -10.0f, -10.0f));

    REQUIRE(box.getMinEdge() == xe::Vector3(-10.0f, -10.0f, -10.0f));
    REQUIRE(box.getCenter() != prevCenter);
    REQUIRE(box.getSize() != prevSize);
}

TEST_CASE("Box isInside checks if a point is between the min and max edges", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};

    REQUIRE(box.isInside({0.0f, 0.0f, 0.0f}));
    REQUIRE(box.isInside({1.0f, 0.0f, 0.0f}));
    REQUIRE(box.isInside({0.0f, 1.0f, 0.0f}));
    REQUIRE(box.isInside({0.0f, 0.0f, 1.0f}));
    REQUIRE(box.isInside({-1.0f, 0.0f, 0.0f}));
    REQUIRE(box.isInside({0.0f, -1.0f, 0.0f}));
    REQUIRE(box.isInside({0.0f, 0.0f, -1.0f}));

    REQUIRE_FALSE(box.isInside({10.0f, 0.0f, 0.0f}));
    REQUIRE_FALSE(box.isInside({0.0f, 10.0f, 0.0f}));
    REQUIRE_FALSE(box.isInside({0.0f, 0.0f, 10.0f}));

    for (int i = xe::Box::MinEdge; i <= xe::Box::MaxEdge; i++) {
        REQUIRE(box.isInside(box.getEdge(i)));
    }
}

TEST_CASE("Box intersect checks if there is overlapping between two boxes", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    xe::Box box2 = xe::Box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};

    REQUIRE(box.intersect(box));
    REQUIRE(box.intersect(box2));
    REQUIRE(box2.intersect(box));

    const xe::Box within{{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};
    REQUIRE(box.intersect(within));
    REQUIRE(within.intersect(box));
}

TEST_CASE("Box intersect checks if there is overlapping with a half-sized box", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};

    const auto size = box.getSize();

    const xe::Vector3 displacements[] = {
        {0.5f * size.x, 0.0f, 0.0f},
        {0.0f, 0.5f * size.y, 0.0f},
        {0.0f, 0.0f, 0.5f * size.z},
        {-0.5f * size.x, 0.0f, 0.0f},
        {0.0f, -0.5f * size.y, 0.0f},
        {0.0f, 0.0f, -0.5f * size.z},
    };

    for (const auto &displacement : displacements) {
        const auto displaced = xe::Box{box.getMinEdge() + displacement, box.getMaxEdge() + displacement};

        REQUIRE(box.intersect(displaced));
        REQUIRE(displaced.intersect(box));
    }
}

TEST_CASE("Box intersect checks if there is overlapping between two boxes with a nearby position", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};

    const auto size = box.getSize();

    const xe::Vector3 displacements[] = {
        {0.99f * size.x, 0.0f, 0.0f},
        {0.0f, 0.99f * size.y, 0.0f},
        {0.0f, 0.0f, 0.99f * size.z},
        {-0.99f * size.x, 0.0f, 0.0f},
        {0.0f, -0.99f * size.y, 0.0f},
        {0.0f, 0.0f, -0.99f * size.z},
    };

    for (const auto &displacement : displacements) {
        const auto displaced = xe::Box{box.getMinEdge() + displacement, box.getMaxEdge() + displacement};

        REQUIRE(box.intersect(displaced));
        REQUIRE(displaced.intersect(box));
    }
}

TEST_CASE("Box intersect checks if there is overlapping between two boxes with exact size apart", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    const auto size = box.getSize();

    const xe::Vector3 displacements[] = {
        {size.x, 0.0f, 0.0f},
        {0.0f, size.y, 0.0f},
        {0.0f, 0.0f, size.z},
        {-size.x, 0.0f, 0.0f},
        {0.0f, -size.y, 0.0f},
        {0.0f, 0.0f, -size.z},
    };

    for (const auto &displacement : displacements) {
        const auto displaced = xe::Box{box.getMinEdge() + displacement, box.getMaxEdge() + displacement};

        REQUIRE_FALSE(box.intersect(displaced));
        REQUIRE_FALSE(displaced.intersect(box));
    }
}

TEST_CASE("Box intersect checks if there is overlapping between two boxes displaced two and a half of the first box size", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    const auto size = box.getSize();

    const xe::Vector3 displacements[] = {
        {2.5f * size.x, 0.0f, 0.0f},
        {0.0f, 2.5f * size.y, 0.0f},
        {0.0f, 0.0f, 2.5f * size.z},
        {-2.5f * size.x, 0.0f, 0.0f},
        {0.0f, -2.5f * size.y, 0.0f},
        {0.0f, 0.0f, -2.5f * size.z},
    };

    for (const auto &displacement : displacements) {
        const auto displaced = xe::Box{box.getMinEdge() + displacement, box.getMaxEdge() + displacement};

        REQUIRE_FALSE(displaced.intersect(box));
        REQUIRE_FALSE(box.intersect(displaced));
    }
}

TEST_CASE("Box operator<< should create a unique string representation from different boxes", "[math][box]") {
    xe::Box box{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    xe::Box box1{{0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}};

    std::stringstream ss;
    ss << box;
    REQUIRE(ss.str() != "");

    std::stringstream ss1;
    ss1 << box1;
    REQUIRE(ss.str() != ss1.str());
}
