#include <glaze/raii.hpp>

#include <cstdio>

// Smoke test: exercise the API-agnostic glaze::Unique<T> + Traits<T>
// customization point without depending on the per-API binding. Defining a
// trivial Traits specialization for an int handle is enough to confirm that
// the package's `glaze::raii` component is exported correctly.
namespace {
struct Counter {
    int value;
};
} // namespace

namespace glaze {
template <>
struct Traits<Counter> {
    static Counter create(int seed) { return Counter{seed}; }
    static void destroy(Counter & /*c*/) {}
    static bool valid(const Counter &c) { return c.value > 0; }
};
} // namespace glaze

int main() {
    auto unique = glaze::makeUnique<Counter>(7);
    const int observed = unique ? unique->value : -1;
    std::printf("glaze-cpp smoke ok (counter=%d)\n", observed);
    return observed == 7 ? 0 : 1;
}
