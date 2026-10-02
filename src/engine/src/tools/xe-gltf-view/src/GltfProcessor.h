
#pragma once

#include <iostream>

#include "GltfUtil.h"
#include "cgltf.h"

namespace xe::gl {
    class RendererGL;
}

class GltfProcessor {
public:
    bool process(const std::string &filePath) {
        std::cout << "Loading " << filePath << std::endl;

        cgltf_options options = {};
        cgltf_data *data = nullptr;
        cgltf_result result = cgltf_parse_file(&options, filePath.c_str(), &data);

        if (result != cgltf_result_success) {
            std::cerr << "CGLTF: Couldn't load file '" << filePath << "'. Error code: " << to_string(result);
            return false;
        }

        result = cgltf_load_buffers(&options, data, filePath.c_str());
        if (result != cgltf_result_success) {
            std::cerr << "CGLTF: error while loading buffers '" << filePath << "'. Error code: " << to_string(result);
            return false;
        }

        std::cout << "Found " << data->lights_count << " lights" << std::endl;
        for (cgltf_size i = 0; i < data->lights_count; i++) {
            process_light(data->lights + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->cameras_count << " cameras" << std::endl;
        for (cgltf_size i = 0; i < data->cameras_count; i++) {
            process_camera(data->cameras + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->meshes_count << " meshes" << std::endl;
        for (cgltf_size i = 0; i < data->meshes_count; i++) {
            process_mesh(data->meshes + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->animations_count << " animations" << std::endl;
        for (cgltf_size i = 0; i < data->animations_count; i++) {
            process_animation(data->animations + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->skins_count << " skins" << std::endl;
        for (cgltf_size i = 0; i < data->skins_count; i++) {
            process_skins(data->skins + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->textures_count << " textures" << std::endl;
        for (cgltf_size i = 0; i < data->textures_count; i++) {
            process_texture(data->textures + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->images_count << " images" << std::endl;
        for (cgltf_size i = 0; i < data->images_count; i++) {
            process_image(data->images + i);
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Found " << data->samplers_count << " samplers" << std::endl;
        for (cgltf_size i = 0; i < data->samplers_count; i++) {
            process_sampler(data->samplers + i);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->accessors_count << " accessors" << std::endl;
        // for (cgltf_size i = 0; i < data->accessors_count; i++) {
        //     process_accessor(data->accessors + i);
        // }
        std::cout << std::endl;

        std::cout << "Found " << data->buffers_count << " buffers" << std::endl;
        for (cgltf_size i = 0; i < data->buffers_count; i++) {
            process_buffer(data->buffers + i);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->buffer_views_count << " buffer views" << std::endl;
        for (cgltf_size i = 0; i < data->buffer_views_count; i++) {
            process_buffer_view(data->buffer_views + i);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->data_extensions_count << " data extensions" << std::endl;
        for (cgltf_size i = 0; i < data->data_extensions_count; i++) {
            process_data_extension(data->data_extensions + i);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->extensions_required_count << " extensions required" << std::endl;
        for (cgltf_size i = 0; i < data->extensions_required_count; i++) {
            process_extensions_required(data->extensions_required[i]);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->extensions_used_count << " extensions used" << std::endl;
        for (cgltf_size i = 0; i < data->extensions_used_count; i++) {
            process_extensions_used(data->extensions_used[i]);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->variants_count << " variants" << std::endl;
        for (cgltf_size i = 0; i < data->variants_count; i++) {
            process_variant(data->variants + i);
        }
        std::cout << std::endl;

        std::cout << "Found " << data->scenes_count << " scenes" << std::endl;
        for (cgltf_size i = 0; i < data->scenes_count; i++) {
            process_scene(data, data->scenes + i);
        }
        std::cout << std::endl;

        cgltf_free(data);

        return true;
    }

private:
    void process_camera(cgltf_camera *camera) {
        std::cout << "Camera node name: " << camera->name << std::endl;
    }

    void process_light(cgltf_light *light) {
        std::cout << "Light node name: " << light->name << std::endl;
    }

    void process_material(const cgltf_material *material) {
        std::cout << "Material name " << material->name << std::endl;

        std::cout << "Material has clearcoat " << evaluate_bool(material->has_clearcoat) << std::endl;
        std::cout << "Material has ior " << evaluate_bool(material->has_ior) << std::endl;
        std::cout << "Material has metallic roughness " << evaluate_bool(material->has_pbr_metallic_roughness) << std::endl;
        std::cout << "Material has specular glossiness " << evaluate_bool(material->has_pbr_specular_glossiness) << std::endl;
        std::cout << "Material has sheen " << evaluate_bool(material->has_sheen) << std::endl;
        std::cout << "Material has specular " << evaluate_bool(material->has_specular) << std::endl;
        std::cout << "Material has transmission " << evaluate_bool(material->has_transmission) << std::endl;
        std::cout << "Material has volume " << evaluate_bool(material->has_volume) << std::endl;
    }

    void process_buffer_view(const cgltf_buffer_view *view) {
        std::cout << "Buffer view name: " << (view->name ? view->name : "<noname>") << std::endl;
        std::cout << "Buffer view type: " << view->type << std::endl;
    }

    void process_accessor(const cgltf_accessor *accessor) {
        std::cout << "Accessor name: " << sanitizeString(accessor->name) << std::endl;
        std::cout << "Accessor count: " << accessor->count << std::endl;
        std::cout << "Accessor type: " << accessor->type << std::endl;
        std::cout << "Accessor component type: " << accessor->component_type << std::endl;
        std::cout << "Accessor extensions count: " << accessor->extensions_count << std::endl;
        std::cout << "Accessor have buffer_view: " << toBool(accessor->buffer_view) << std::endl;
        std::cout << "Accessor has max: " << toBool(accessor->has_max) << std::endl;
        std::cout << "Accessor has min: " << toBool(accessor->has_min) << std::endl;
        std::cout << "Accessor is sparse: " << toBool(accessor->is_sparse) << std::endl;
        std::cout << "Accessor normalized: " << toBool(accessor->normalized) << std::endl;

        if (accessor->buffer_view) {
            process_buffer_view(accessor->buffer_view);
        }
    }

    void process_attribute(const cgltf_attribute &attribute) {
        std::cout << "Attribute name " << attribute.name << std::endl;
        std::cout << "Attribute index " << attribute.index << std::endl;
        std::cout << "Attribute type " << attribute.type << std::endl;
        process_accessor(attribute.data);
    }

    // process a single mesh primitive
    void process_primitive(const cgltf_primitive &primitive) {
        std::cout << "Primitive type " << primitive.type << std::endl;
        std::cout << "Primitive attribute count  " << primitive.attributes_count << std::endl;
        std::cout << "Primitive has indices " << toBool(primitive.indices) << std::endl;
        std::cout << "Primitive has material " << toBool(primitive.material) << std::endl;
        std::cout << "Primitive mappings count " << primitive.mappings_count << std::endl;
        std::cout << "Primitive targets count " << primitive.targets_count << std::endl;
        std::cout << "Primitive has draco mesh compression " << toBool(primitive.has_draco_mesh_compression) << std::endl;
        std::cout << "Primitive extension count " << primitive.extensions_count << std::endl;

        if (primitive.indices) {
            const auto indexAccessor = primitive.indices;
            const auto indicesOffset = indexAccessor->buffer_view->offset;
            const auto indicesSize = indexAccessor->buffer_view->size;

            std::cout << "Primitive indices offset " << indicesOffset << std::endl;
            std::cout << "Primitive indices size " << indicesSize << std::endl;
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
            std::cout << "Primitive morph target " << i << " attribute count: " << target->attributes_count << std::endl;

            for (cgltf_size j = 0; j < target->attributes_count; j++) {
                std::cout << "Primitive morph target " << i << " attribute " << j << " name: " << target->attributes[j].name << std::endl;
            }
        }
    }

    void process_mesh(cgltf_mesh *mesh) {
        std::cout << "Node mesh " << mesh->name << std::endl;
        std::cout << "Node primitives count " << mesh->primitives_count << std::endl;

        for (cgltf_size i = 0; i < mesh->primitives_count; i++) {
            process_primitive(mesh->primitives[i]);
        }
    }

    void process_node(cgltf_node *node) {
        std::cout << "Node name" << node->name << std::endl;
        std::cout << "Node has matrix " << node->has_matrix << std::endl;
        std::cout << "Node has translation " << node->has_translation << std::endl;
        std::cout << "Node has rotation " << node->has_rotation << std::endl;
        std::cout << "Node has scale " << node->has_scale << std::endl;

        if (node->mesh) {
            process_mesh(node->mesh);
        }

        if (node->light) {
            process_light(node->light);
        }

        if (node->camera) {
            process_camera(node->camera);
        }

        std::cout << "Node children " << node->children_count << std::endl;
        for (cgltf_size ci = 0; ci < node->children_count; ci++) {
            process_node(node->children[ci]);
        }
    }

    void process_scene(cgltf_data *data, cgltf_scene *scene) {
        std::cout << "Scene name " << scene->name << std::endl;
        std::cout << "Scene node count " << scene->nodes_count << std::endl;

        for (cgltf_size ni = 0; ni < scene->nodes_count; ni++) {
            process_node(data->nodes + ni);
        }
    }

    void process_animation(cgltf_animation *animation) {
        std::cout << "Animation name: " << evaluate_name(animation->name) << std::endl;
        std::cout << "Animation samplers count: " << animation->samplers_count << std::endl;
        std::cout << "Animation channels count: " << animation->channels_count << std::endl;
        std::cout << "Animation extensions count: " << animation->extensions_count << std::endl;
    }

    void process_skins(cgltf_skin *skin) {
        std::cout << "Skin name: " << evaluate_name(skin->name) << std::endl;
        std::cout << evaluate_ptr("Skin has skeleton node: ", skin->skeleton) << std::endl;
        std::cout << "Animation joints count: " << skin->joints_count << std::endl;
        std::cout << "Animation extensions count: " << skin->extensions_count << std::endl;
    }

    void process_texture(cgltf_texture *texture) {
        std::cout << "Texture name: " << (texture->name ? texture->name : "<noname>") << std::endl;
        std::cout << "Texture extensions count: " << texture->extensions_count << std::endl;
        std::cout << "Texture has sampler: " << (texture->sampler ? "true" : "false") << std::endl;

        const cgltf_sampler *sampler = texture->sampler;

        if (sampler) {
            std::cout << "Texture sampler name: " << (sampler->name ? sampler->name : "<noname>") << std::endl;
            std::cout << "Texture mag filter: " << sampler->mag_filter << std::endl;
            std::cout << "Texture min filter: " << sampler->min_filter << std::endl;
            std::cout << "Texture wrap s: " << sampler->wrap_s << std::endl;
            std::cout << "Texture wrap t: " << sampler->wrap_t << std::endl;
            std::cout << "Texture extensions count: " << sampler->extensions_count << std::endl;
        }

        std::cout << "Texture has basisu: " << (texture->has_basisu ? "true" : "false") << std::endl;
    }

    void process_image(cgltf_image *image) {
        std::cout << "Image name: " << (image->name ? image->name : "<noname>") << std::endl;
        std::cout << "Image uri: " << (image->uri ? image->uri : "<noname>") << std::endl;
        std::cout << "Image mime type: " << (image->mime_type ? image->mime_type : "<noname>") << std::endl;
        std::cout << "Image has buffer view: " << (image->buffer_view ? "true" : "false") << std::endl;
        std::cout << "Image extensions count: " << image->extensions_count << std::endl;
    }

    void process_sampler(cgltf_sampler *sampler) {
        std::cout << "Image name: " << (sampler->name ? sampler->name : "<noname>") << std::endl;
    }

    void process_buffer(cgltf_buffer *buffer) {
        std::cout << "Buffer name: " << sanitizeString(buffer->name) << std::endl;
        std::cout << "Buffer uri: " << sanitizeString(buffer->uri) << std::endl;
        std::cout << "Buffer size: " << (static_cast<float>(buffer->size) / 1024.0f / 1024.0f) << " MB" << std::endl;
    }

    void process_data_extension(cgltf_extension *extension) {
        std::cout << "Data extension name" << extension->name << std::endl;
    }

    void process_extensions_required(char *str) {
        std::cout << "Extension required " << str << std::endl;
    }

    void process_extensions_used(char *str) {
        std::cout << "Extension used " << str << std::endl;
    }

    void process_variant(cgltf_material_variant *material_variant) {
        std::cout << "Material variant name: " << material_variant->name << std::endl;
    }
};
