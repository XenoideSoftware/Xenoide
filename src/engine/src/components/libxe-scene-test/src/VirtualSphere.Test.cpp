
/*
#include <catch2/catch_all.hpp>

#include <xe/Scene/VirtualSphere.h>

TEST_CASE("VirtualSphere accessors should fill its fields properly", "[VirtualSphere]") {
    SECTION("Given virtual spheres with standard sizes") {
        auto vs1 = xe::VirtualSphere{{640, 480}};
        auto vs2 = xe::VirtualSphere{{800, 600}};

        SECTION("Their sizes should match with those supplied in the constructor") {
            vs1.setScreenSize({1024, 768});
            vs2.setScreenSize({1360, 1024});

            REQUIRE(vs1.getScreenSize() == xe::Vector2{1024, 768});
            REQUIRE(vs2.getScreenSize() == xe::Vector2{1360, 1024});
        }

        SECTION("Their sizes should match with those supplied in the setters") {
            REQUIRE(vs1.getScreenSize() == xe::Vector2{640, 480});
            REQUIRE(vs2.getScreenSize() == xe::Vector2{800, 600});
        }
    }
}

TEST_CASE("VirtualSphere computePointAt method should return points lying into the surface of the sphere", "[VirtualSphere]") {
    SECTION("Given virtual spheres with standard sizes") {
        xe::VirtualSphere spheres[] = {
            {{100, 100}},
            {{320, 200}},
            {{200, 320}},
        };

        SECTION("should compute the coordinates {-0.707106769, -0.707106769, 0} value at the Left-Bottom edge") {
            REQUIRE(spheres[0].computePointAt({0, 0}) == xe::Vector3{-0.707106769f, -0.707106769f, 0.0f});
            REQUIRE(spheres[1].computePointAt({0, 0}) == xe::Vector3{-0.707106769f, -0.707106769f, 0.0f});
            REQUIRE(spheres[2].computePointAt({0, 0}) == xe::Vector3{-0.707106769f, -0.707106769f, 0.0f});
        }

        SECTION("should compute the coordinates {0.707106769, 0.707106769, 0} value at the right-top edge") {
            REQUIRE(spheres[0].computePointAt({100, 100}) == xe::Vector3{0.707106769f, 0.707106769f, 0.0f});
            REQUIRE(spheres[1].computePointAt({320, 200}) == xe::Vector3{0.707106769f, 0.707106769f, 0.0f});
            REQUIRE(spheres[2].computePointAt({200, 320}) == xe::Vector3{0.707106769f, 0.707106769f, 0.0f});
        }

        SECTION("should compute the coordinates {0, 0, 1} value at the center of the screen") {
            REQUIRE(spheres[0].computePointAt({50, 50}) == xe::Vector3{0.0f, 0.0f, 1.0f});
            REQUIRE(spheres[1].computePointAt({160, 100}) == xe::Vector3{0.0f, 0.0f, 1.0f});
            REQUIRE(spheres[2].computePointAt({100, 160}) == xe::Vector3{0.0f, 0.0f, 1.0f});
        }
    }
}

TEST_CASE("VirtualSphere comparison operators should compare sizes", "[VirtualSphere]") {
    SECTION("Given virtual spheres with standard sizes") {
        auto vs1 = xe::VirtualSphere{{640, 480}};
        auto vs2 = xe::VirtualSphere{{800, 600}};
        auto vs3 = xe::VirtualSphere{{640, 480}};
        auto vs4 = xe::VirtualSphere{{800, 600}};

        SECTION("equality operator should compare the sizes of the virtual sphere") {
            REQUIRE(vs1 == vs3);
            REQUIRE(vs3 == vs1);

            REQUIRE(vs2 == vs4);
            REQUIRE(vs4 == vs2);
        }

        SECTION("inequality operator should compare the sizes of the virtual sphere") {
            REQUIRE(vs1 != vs2);
            REQUIRE(vs2 != vs1);

            REQUIRE(vs3 != vs4);
            REQUIRE(vs4 != vs3);
        }
    }
}
*/
