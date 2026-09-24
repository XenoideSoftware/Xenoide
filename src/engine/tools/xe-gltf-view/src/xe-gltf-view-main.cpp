
#include "GltfDataLoader.h"
#include "GltfRenderer.h"
#include "Window.h"

#include <iostream>
#include <bpstd/span.hpp>

struct GltfViewParams {
    std::string gltfPath;
};

GltfViewParams parseArgs(bpstd::span<char *> args) {
    GltfViewParams result;

    if (args.size() < 2) {
        throw std::runtime_error("Usage: xe-gltf-view <gltf-file>");
    }

    result.gltfPath = args[1];

    return result;
}

int gltf_view_main(const bpstd::span<char *> &args) {
    using xe::gltf_view::GltfRenderer;

    const GltfViewParams params = parseArgs(args);

    Window window;
    window.initialize();

    GltfRenderer renderer{params.gltfPath};
    window.setData(renderer.getData());

    while (window.pollInput()) {
        window.update();
        renderer.beginFrame();

        window.prepareUI();
        // render the geometries
        renderer.render();

        // render the UI
        window.drawUI();

        // present the frame
        renderer.endFrame();
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    const bpstd::span<char *> args{argv, static_cast<size_t>(argc)};

    return gltf_view_main(args);

    try {
        return gltf_view_main(args);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
