// xe-gltfc-main.cpp
// Simple tool: import any 3D model with Assimp and export as glTF 2.0 binary (.glb).
// Build requires Assimp (importer + exporter components).
// Usage: xe-gltfc <input-model> [output.glb]

#include <assimp/DefaultLogger.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>

static void setupAssimpLogger() {
    using namespace Assimp;
    if (!DefaultLogger::isNullLogger())
        return;
    DefaultLogger::create("", Logger::VERBOSE, aiDefaultLogStream_STDOUT);
}

int main(int argc, char **argv) {
    std::string inputPath = "/Users/fapablaza/Dropbox/GameDev/Capybaria/raw-assets/models/capybara-02/Capybara.fbx";
    std::string outputPath = "/Users/fapablaza/Dropbox/GameDev/Capybaria/raw-assets/models/capybara-02/Capybara.glb";

    // std::string inputPath = argv[1];
    // std::string outputPath = (argc >= 3) ? argv[2] : deriveOutputPath(inputPath);

    if (!std::filesystem::exists(inputPath)) {
        std::cerr << "Error: input file does not exist: " << inputPath << "\n";
        return 2;
    }

    setupAssimpLogger();

    Assimp::Importer importer;

    // Optional: configure importer properties here if needed
    // Example: importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    unsigned int ppFlags = aiProcess_Triangulate |
                           // aiProcess_GenSmoothNormals |
                           // aiProcess_ImproveCacheLocality |
                           aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
                           // aiProcess_CalcTangentSpace |
                           // aiProcess_GenUVCoords |
                           aiProcess_SortByPType |
                           // aiProcess_ValidateDataStructure |
                           aiProcess_RemoveRedundantMaterials; // |
    // aiProcess_FixInfacingNormals |
    // aiProcess_FindInvalidData;

    const aiScene *scene = importer.ReadFile(inputPath, ppFlags);
    if (!scene) {
        std::cerr << "Import failed: " << importer.GetErrorString() << "\n";
        return 3;
    }

    if (scene->mNumMeshes == 0) {
        std::cerr << "Error: scene has no meshes after processing.\n";
        return 4;
    }

    Assimp::Exporter exporter;

    // List available export formats if needed (uncomment for debugging)
    /*
    std::cout << "Available export formats:\n";
    for (size_t i = 0; i < exporter.GetExportFormatCount(); ++i) {
        const aiExportFormatDesc* d = exporter.GetExportFormatDescription(i);
        std::cout << "  " << d->id << " (" << d->fileExtension << "): " << d->description << "\n";
    }
    */

    // glTF2 binary exporter id is "glb2" (ASCII glTF2 is "gltf2")
    const char *targetFormatId = "glb2";

    // Ensure output directory exists
    try {
        std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    } catch (...) {
        std::cerr << "Unknown error while create directories" << '\n';
    }

    aiReturn ret = exporter.Export(scene, targetFormatId, outputPath, 0);
    if (ret != aiReturn_SUCCESS) {
        std::cerr << "Export failed: " << exporter.GetErrorString() << "\n";
        return 5;
    }

    std::cout << "Exported GLB successfully: " << outputPath << "\n";
    return 0;
}
