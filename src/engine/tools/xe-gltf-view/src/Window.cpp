
#include "Window.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <bpstd/span.hpp>
#include <fmt/format.h>

#include "GltfUtil.h"
#include "cgltf.h"
#include "xe/gl/RendererGL.h"
#include <glaze/gl.h>

#include "bindings/imgui_impl_opengl3.h"
#include "bindings/imgui_impl_sdl2.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

namespace xe::gl {
    class RendererGL;
}

static const char *stringOr(const char *value, const char *defaultValue = "<null>") {
    assert(defaultValue != nullptr);
    return value ? value : defaultValue;
}

void process_camera(cgltf_camera *camera) {
    std::cout << "Camera node " << camera->name << '\n';
}

void process_light(cgltf_light *light) {
    std::cout << "Light node" << light->name << '\n';
}

void process_material(const cgltf_material *material) {
    std::cout << "Material name " << material->name << '\n';

    std::cout << "Material has clearcoat " << evaluate_bool(material->has_clearcoat) << '\n';
    std::cout << "Material has ior " << evaluate_bool(material->has_ior) << '\n';
    std::cout << "Material has metallic roughness " << evaluate_bool(material->has_pbr_metallic_roughness) << '\n';
    std::cout << "Material has specular glossiness " << evaluate_bool(material->has_pbr_specular_glossiness) << '\n';
    std::cout << "Material has sheen " << evaluate_bool(material->has_sheen) << '\n';
    std::cout << "Material has specular " << evaluate_bool(material->has_specular) << '\n';
    std::cout << "Material has transmission " << evaluate_bool(material->has_transmission) << '\n';
    std::cout << "Material has volume " << evaluate_bool(material->has_volume) << '\n';
}

void process_buffer_view(const cgltf_buffer_view *view) {
    std::cout << "Buffer view name: " << (view->name ? view->name : "<noname>") << '\n';
    std::cout << "Buffer view type: " << view->type << '\n';
}

void process_accessor(const cgltf_accessor *accessor) {
    std::cout << "Accessor name: " << sanitizeString(accessor->name) << '\n';
    std::cout << "Accessor count: " << accessor->count << '\n';
    std::cout << "Accessor type: " << accessor->type << '\n';
    std::cout << "Accessor component type: " << accessor->component_type << '\n';
    std::cout << "Accessor extensions count: " << accessor->extensions_count << '\n';
    std::cout << "Accessor have buffer_view: " << toBool(accessor->buffer_view) << '\n';
    std::cout << "Accessor has max: " << toBool(accessor->has_max) << '\n';
    std::cout << "Accessor has min: " << toBool(accessor->has_min) << '\n';
    std::cout << "Accessor is sparse: " << toBool(accessor->is_sparse) << '\n';
    std::cout << "Accessor normalized: " << toBool(accessor->normalized) << '\n';

    if (accessor->buffer_view) {
        process_buffer_view(accessor->buffer_view);
    }
}

void process_attribute(const cgltf_attribute &attribute) {
    std::cout << "Attribute name " << attribute.name << '\n';
    std::cout << "Attribute index " << attribute.index << '\n';
    std::cout << "Attribute type " << attribute.type << '\n';
    process_accessor(attribute.data);
}

// process a single mesh primitive
void process_primitive(const cgltf_primitive &primitive) {
    std::cout << "Primitive type " << primitive.type << '\n';
    std::cout << "Primitive attribute count  " << primitive.attributes_count << '\n';
    std::cout << "Primitive has indices " << toBool(primitive.indices) << '\n';
    std::cout << "Primitive has material " << toBool(primitive.material) << '\n';
    std::cout << "Primitive mappings count " << primitive.mappings_count << '\n';
    std::cout << "Primitive targets count " << primitive.targets_count << '\n';
    std::cout << "Primitive has draco mesh compression " << toBool(primitive.has_draco_mesh_compression) << '\n';
    std::cout << "Primitive extension count " << primitive.extensions_count << '\n';

    if (primitive.indices) {
        const auto indexAccessor = primitive.indices;
        const auto indicesOffset = indexAccessor->buffer_view->offset;
        const auto indicesSize = indexAccessor->buffer_view->size;

        std::cout << "Primitive indices offset " << indicesOffset << '\n';
        std::cout << "Primitive indices size " << indicesSize << '\n';
        // process_accessor(primitive.indices);
    }

    /*
    for (cgltf_size i = 0; i < primitive.attributes_count; i++) {
        const auto attribute = primitive.attributes[i];
        const cgltf_accessor *accessor = attribute.data;

        std::cout << "Attribute name " << attribute.name << std::endl;
        std::cout << "Attribute index " << attribute.index << std::endl;
        std::cout << "Attribute type " << attribute.type << std::endl;

        std::cout << "Attribute accessor address: " << accessor << std::endl;
        std::cout << "Attribute accessor name: " << toStr(accessor->name) << std::endl;
        std::cout << "Attribute accessor count: " << accessor->count << std::endl;
        std::cout << "Attribute accessor type: " << accessor->type << std::endl;
        std::cout << "Attribute accessor component type: " << accessor->component_type << std::endl;
        std::cout << "Attribute accessor extensions count: " << accessor->extensions_count << std::endl;
        std::cout << "Attribute accessor have buffer_view: " << toBool(accessor->buffer_view) << std::endl;
        std::cout << "Attribute accessor has max: " << toBool(accessor->has_max) << std::endl;
        std::cout << "Attribute accessor has min: " << toBool(accessor->has_min) << std::endl;
        std::cout << "Attribute accessor is sparse: " << toBool(accessor->is_sparse) << std::endl;
        std::cout << "Attribute accessor normalized: " << toBool(accessor->normalized) << std::endl;

        if (accessor->buffer_view) {
            const auto view = accessor->buffer_view;

            std::cout << "Buffer view address: " << view << std::endl;
            std::cout << "Buffer view name: " << toStr(view->name) << std::endl;
            std::cout << "Buffer view type: " << view->type << std::endl;
            std::cout << "Buffer view offset, size: " << view->offset << ", " << view->size << std::endl;
            std::cout << "Buffer view stride: " << view->stride << std::endl;
        }
    }
    */

    if (primitive.material) {
        process_material(primitive.material);
    }

    for (cgltf_size i = 0; i < primitive.targets_count; i++) {
        auto target = primitive.targets + i;
        std::cout << "Primitive morph target " << i << " attribute count: " << target->attributes_count << '\n';

        for (cgltf_size j = 0; j < target->attributes_count; j++) {
            std::cout << "Primitive morph target " << i << " attribute " << j << " name: " << target->attributes[j].name << '\n';
        }
    }
}

void process_mesh(cgltf_mesh *mesh) {
    std::cout << "Node mesh " << mesh->name << '\n';
    std::cout << "Node primitives count " << mesh->primitives_count << '\n';

    for (cgltf_size i = 0; i < mesh->primitives_count; i++) {
        process_primitive(mesh->primitives[i]);
    }
}

void process_sampler(cgltf_sampler *sampler) {
    std::cout << "Sampler name: " << sampler->name << '\n';
}

void process_buffer(cgltf_buffer *buffer) {
    std::cout << "Buffer name: " << sanitizeString(buffer->name) << '\n';
    std::cout << "Buffer uri: " << sanitizeString(buffer->uri) << '\n';
    std::cout << "Buffer size: " << (static_cast<float>(buffer->size) / 1024.0f / 1024.0f) << " MB" << '\n';
}

void process_data_extension(cgltf_extension *extension) {
    std::cout << "Data extension name" << extension->name << '\n';
}

void process_extensions_required(char *str) {
    std::cout << "Extension required " << str << '\n';
}

void process_extensions_used(char *str) {
    std::cout << "Extension used " << str << '\n';
}

void process_variant(cgltf_material_variant *material_variant) {
    std::cout << "Material variant name: " << material_variant->name << '\n';
}

std::string itemLabel(const std::string &label, std::optional<size_t> i = {}) {
    if (!i.has_value()) {
        return label;
    }

    return fmt::format("{} {}", label, i.value());
}

template <typename Func> void treeNode(const std::string &label, Func func) {
    const ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::TreeNodeEx(label.c_str(), flag)) {
        func();
        ImGui::TreePop();
    }
}

template <typename T> void renderTreeNode(const std::string &label, const bpstd::span<T> &values) {
    const ImGuiTreeNodeFlags flag = values.size() > 1 ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::TreeNodeEx(label.c_str(), flag)) {
        for (size_t i = 0; i < values.size(); i++) {
            const T &value = values[i];
            renderValue(value, i);
        }
        ImGui::TreePop();
    }
}

void renderValue(cgltf_node *node, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Node", i).c_str())) {
        ImGui::Text("Name: %s", stringOr(node->name));
        ImGui::Text("Has matrix: %s", (node->has_matrix ? "true" : "false"));
        ImGui::Text("Has translation: %s", (node->has_translation ? "true" : "false"));
        ImGui::Text("Has rotation: %s", (node->has_rotation ? "true" : "false"));
        ImGui::Text("Has scale: %s", (node->has_scale ? "true" : "false"));
        ImGui::Text("Weights count: %d", node->weights_count);

        /*
        if (node->mesh) {
            process_mesh(node->mesh);
        }

        if (node->light) {
            process_light(node->light);
        }

        if (node->camera) {
            process_camera(node->camera);
        }
        */

        if (node->children_count) {
            bpstd::span<cgltf_node *> nodes{node->children, node->children_count};
            renderTreeNode(fmt::format("Children ({})", nodes.size()), nodes);
        } else {
            ImGui::Text("Children (0)");
        }

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_scene &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Scene", i).c_str())) {
        ImGui::Text("Name: %s", (stringOr(value.name)));
        ImGui::Text("Extensions count: %d", value.extensions_count);

        bpstd::span<cgltf_node *> nodes{value.nodes, value.nodes_count};
        renderTreeNode(fmt::format("Children nodes ({})", nodes.size()), nodes);

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_light &light, std::optional<size_t> i = {}) {
    ImGui::Text(stringOr(light.name, "<noname>"));
}

void renderValue(const cgltf_camera &camera, std::optional<size_t> i = {}) {
    ImGui::Text(stringOr(camera.name, "<noname>"));
}

void renderValue(const cgltf_mesh &mesh, std::optional<size_t> i = {}) {
    ImGui::Text(stringOr(mesh.name, "<noname>"));
}

void renderValue(const cgltf_animation &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Animation", i).c_str())) {
        ImGui::Text("Name: %s", (stringOr(value.name)));
        ImGui::Text("Samplers count: %d", value.samplers_count);
        ImGui::Text("Channels count: %d", value.channels_count);
        ImGui::Text("Extensions count: %d", value.extensions_count);

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_skin &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Skin", i).c_str())) {
        ImGui::Text("Name: %s", (stringOr(value.name)));
        ImGui::Text("Has skeleton?: %s", value.skeleton ? "true" : "false");
        ImGui::Text("Joint count: %d", value.joints_count);
        ImGui::Text("Extensions count: %d", value.extensions_count);

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_sampler &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Sampler", i).c_str())) {
        ImGui::Text("Sampler name: %s", (stringOr(value.name)));
        ImGui::Text("Sampler mag filter: %d", value.mag_filter);
        ImGui::Text("Sampler min filter: %d", value.min_filter);
        ImGui::Text("Sampler wrap s: %d", value.wrap_s);
        ImGui::Text("Sampler wrap t: %d", value.wrap_t);
        ImGui::Text("Sampler extensions count: %d", value.extensions_count);

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_texture &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Texture", i).c_str())) {
        ImGui::Text("Name: %s", stringOr(value.name));
        ImGui::Text("Extensions count: %d", value.extensions_count);
        ImGui::Text("Has BasisU: %s", value.has_basisu ? "true" : "false");
        ImGui::Text("Sampler: %s", value.sampler ? "true" : "false");

        if (value.sampler) {
            renderValue(*value.sampler);
        }

        ImGui::TreePop();
    }
}

void renderValue(const cgltf_image &value, std::optional<size_t> i = {}) {
    if (ImGui::TreeNodeEx(itemLabel("Image", i).c_str())) {
        ImGui::Text("Name: %s", stringOr(value.name));
        ImGui::Text("URI: %s", stringOr(value.uri));
        ImGui::Text("MIME type: %s", stringOr(value.mime_type));
        ImGui::Text("Extensions count: %d", value.extensions_count);
        ImGui::Text("Buffer view: %s", value.buffer_view ? "true" : "false");
        ImGui::TreePop();
    }
}

void visitData(const cgltf_data &data) {
    bpstd::span<cgltf_scene> scenes{data.scenes, data.scenes_count};
    renderTreeNode(fmt::format("Scenes ({})", scenes.size()), scenes);

    bpstd::span<cgltf_light> lights{data.lights, data.lights_count};
    renderTreeNode(fmt::format("Lights ({})", lights.size()), lights);

    bpstd::span<cgltf_camera> cameras{data.cameras, data.cameras_count};
    renderTreeNode(fmt::format("Cameras ({})", cameras.size()), cameras);

    bpstd::span<cgltf_mesh> meshes{data.meshes, data.meshes_count};
    renderTreeNode(fmt::format("Meshes ({})", meshes.size()), meshes);

    bpstd::span<cgltf_animation> animations{data.animations, data.animations_count};
    renderTreeNode(fmt::format("Animations ({})", animations.size()), animations);

    bpstd::span<cgltf_skin> skins{data.skins, data.skins_count};
    renderTreeNode(fmt::format("Skins ({})", skins.size()), skins);

    bpstd::span<cgltf_texture> textures{data.textures, data.textures_count};
    renderTreeNode(fmt::format("Textures ({})", textures.size()), textures);

    bpstd::span<cgltf_image> images{data.images, data.images_count};
    renderTreeNode(fmt::format("Images ({})", images.size()), images);
}

Window::Window() {
}

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Window::initialize() {
    if (initializeSDL()) {
        return false;
    }

    if (initializeOpenGL()) {
        return false;
    }

    context = ImGui::CreateContext();
    if (!context) {
        return false;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    return true;
}

int Window::initializeSDL() {
    std::printf("Initializing SDL video subsystem\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::printf("Error while initialising SDL. SDL_Error: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    std::printf("Creating diplay window with 640 x 480 mode, windowed mode\n");
    const Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    window = SDL_CreateWindow("Capybaria", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, flags);
    if (window == nullptr) {
        std::printf("Error while creating Window. SDL_Error: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    return 0;
}

int Window::initializeOpenGL() {
    // OpenGL context configuration
    const int majorVersion = 4;
    const int minorVersion = 1;

    std::map<SDL_GLattr, int> sdlGlAttributes = {
        {SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion},
        {SDL_GL_CONTEXT_MINOR_VERSION, minorVersion},
        {SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE},
        {SDL_GL_RED_SIZE, 8},
        {SDL_GL_GREEN_SIZE, 8},
        {SDL_GL_BLUE_SIZE, 8},
        {SDL_GL_ALPHA_SIZE, 8},
        {SDL_GL_DEPTH_SIZE, 16},
        {SDL_GL_BUFFER_SIZE, 32},
        {SDL_GL_DOUBLEBUFFER, 1}
    };

    for (const auto &pair : sdlGlAttributes) {
        SDL_GL_SetAttribute(pair.first, pair.second);
    }

    std::printf("Requested OpenGL context %d.%d\n", majorVersion, minorVersion);

    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        std::printf("Error while creating OpenGL context. SDL_Error: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    const int makeCurrentResult = SDL_GL_MakeCurrent(window, context);
    if (makeCurrentResult < 0) {
        std::printf("Error while making OpenGL context current. SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    return 0;
}

bool Window::pollInput() {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);

        if (e.type == SDL_QUIT) {
            return false;
        }
    }

    return true;
}

void Window::update() {
}

void Window::prepareUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Window::drawUI() {
    ImGui::Begin("GLTF Tree");
    if (data) {
        treeNode("root", [this]() { visitData(*data); });
    }
    ImGui::End();

    ImGui::Begin("Demo window");
    ImGui::Text("Hello world!");
    if (ImGui::Button("Hello!")) {
        ImGui::Text("Hello world!");
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

void Window::setData(cgltf_data *newData) {
    data = newData;
}
