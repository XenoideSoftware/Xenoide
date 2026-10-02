importStage = component "Import Stage" "Any source format to Compiler IR via cgltf, OpenFBX, tinyobjloader, stb_image, tinyexr." "Stage 1" {
    tags "Component"
}

compilerIR = component "Compiler IR" "Neutral intermediate representation — all importers produce it, all exporters consume it." "Internal" {
    tags "Component"
}

processStage = component "Process Stage" "Mesh optimization (meshoptimizer), texture compression (Basis/libsquish), platform tiling, LOD/mip generation, material baking, vertex quantization." "Stage 2" {
    tags "Component"
}

exportStage = component "Export Stage" "Processed IR to platform-native .xemesh / .xetex / .xeanim binaries." "Stage 3" {
    tags "Component"
}

runtimeLoader = component "Runtime Loader" "Thin mmap/fread loop — maps compiled binary, issues create_buffer/create_texture/create_sampler/create_pipeline." "Loader" {
    tags "Component"
}
