#include "AssimpImporter.hpp"

#include "DefaultMaterial.hpp"
#include "Material.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/pbrmaterial.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"

namespace MapleLeaf {
enum class ImportMode
{
    Default,
    OBJ,
    GLTF2,
};

struct TextureMapping
{
    aiTextureType         aiType;
    unsigned int          aiIndex;
    Material::TextureSlot targetType;
};

static const std::vector<TextureMapping> kTextureMappings[3] = {
    // Default mappings
    {
        {aiTextureType_DIFFUSE, 0},
        {aiTextureType_SPECULAR, 0},
        {aiTextureType_EMISSIVE, 0},
        {aiTextureType_NORMALS, 0},
    },
    // OBJ mappings
    {
        {aiTextureType_DIFFUSE, 0},
        {aiTextureType_SPECULAR, 0},
        {aiTextureType_EMISSIVE, 0},
        // OBJ does not offer a normal map, thus we use the bump map instead.
        {aiTextureType_HEIGHT, 0},
        {aiTextureType_DISPLACEMENT},
    },
    // GLTF2 mappings
    {
        {aiTextureType_DIFFUSE, 0, Material::TextureSlot::BaseColor},
        {aiTextureType_NORMALS, 0, Material::TextureSlot::Normal},
        // GLTF2 exposes metallic roughness texture.
        {AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, Material::TextureSlot::Material},
    }};

class ImporterData
{
public:
    std::filesystem::path path;
    const aiScene*        pScene;
    SceneBuilder&         builder;

    std::unordered_map<uint32_t, std::unique_ptr<Material>> materialMap;

    ImporterData(const std::filesystem::path& path, const aiScene* pAiScene, SceneBuilder& builder)
        : path(path)
        , pScene(pAiScene)
        , builder(builder)
    {}

    void addAiNode(const aiNode* pNode)
    {
        if (mAiNodes.find(pNode->mName.C_Str()) == mAiNodes.end()) {
            mAiNodes[pNode->mName.C_Str()] = {};
        }
        mAiNodes[pNode->mName.C_Str()].push_back(pNode);
    }
    uint32_t getNodeInstanceCount(const std::string& nodeName) const { return (uint32_t)mAiNodes.at(nodeName).size(); }

private:
    std::map<const std::string, std::vector<const aiNode*>> mAiNodes;
};

void loadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath, std::unique_ptr<Material>& pMaterial,
                  ImportMode importMode);
void createAllMaterials(ImporterData& data, const std::filesystem::path& searchPath, ImportMode importMode);
std::unique_ptr<Material> createMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                         ImportMode importMode);

void AssimpImporter::import(const std::filesystem::path& path, SceneBuilder& builder)
{
#ifdef MAPLELEAF_DEBUG
    auto debugStart = Time::Now();
#endif

    if (!std::filesystem::exists(path)) {
        Log::Error("Failed to open file ", path);
        return;
    }

    uint32_t assimpFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs | aiProcess_RemoveComponent | aiProcess_Triangulate;

    assimpFlags &= ~(aiProcess_CalcTangentSpace);           // Never use Assimp's tangent gen code
    assimpFlags &= ~(aiProcess_FindDegenerates);            // Avoid converting degenerated triangles to lines
    assimpFlags &= ~(aiProcess_OptimizeGraph);              // Never use as it doesn't handle transforms with negative determinants
    assimpFlags &= ~(aiProcess_RemoveRedundantMaterials);   // Avoid merging materials as it doesn't load all fields we care about, we merge in
                                                            // 'SceneBuilder' instead.
    assimpFlags &= ~(aiProcess_SplitLargeMeshes);           // Avoid splitting large meshes
    // assimpFlags &= ~aiProcess_OptimizeMeshes;               // Avoid merging original meshes

    int removeFlags = aiComponent_COLORS;
    for (uint32_t uvLayer = 1; uvLayer < AI_MAX_NUMBER_OF_TEXTURECOORDS; uvLayer++) removeFlags |= aiComponent_TEXCOORDSn(uvLayer);
    removeFlags |= aiComponent_TANGENTS_AND_BITANGENTS;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);

    const aiScene* pScene = importer.ReadFile(path.string().c_str(), assimpFlags);
    if (!pScene) Log::Error("Failed to open scene: ", importer.GetErrorString());

    ImporterData data(path, pScene, builder);

    auto searchPath = path.parent_path();

    ImportMode importMode = ImportMode::Default;
    if (path.extension() == "obj") importMode = ImportMode::OBJ;
    if (path.extension() == "gltf" || path.extension() == "glb") importMode = ImportMode::GLTF2;
}

void loadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath, std::unique_ptr<Material>& pMaterial,
                  ImportMode importMode)
{
    const auto& textureMappings = kTextureMappings[int(importMode)];

    switch (importMode) {
    case ImportMode::GLTF2:
        for (const auto& source : textureMappings) {
            if (pAiMaterial->GetTextureCount(source.aiType) < source.aiIndex + 1) continue;

            aiString aiPath;
            pAiMaterial->GetTexture(source.aiType, source.aiIndex, &aiPath);
            std::string path(aiPath.data);
            std::replace(path.begin(), path.end(), '\\', '/');
            if (path.empty()) {
                Log::Warning("AssimpImporter: Texture has empty file name, ignoring.");
                continue;
            }
            auto fullPath = searchPath / path;
            data.builder.loadMaterialTexture(pMaterial, source.targetType, fullPath);
        }
        break;
    case ImportMode::OBJ: break;       // TODO
    case ImportMode::Default: break;   // TODO
    }
}

void createAllMaterials(ImporterData& data, const std::filesystem::path& searchPath, ImportMode importMode)
{
    for (uint32_t i = 0; i < data.pScene->mNumMaterials; i++) {
        const aiMaterial* pAiMaterial = data.pScene->mMaterials[i];
        data.materialMap[i]           = std::move(createMaterial(data, pAiMaterial, searchPath, importMode));
    }
}

std::unique_ptr<Material> createMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                         ImportMode importMode)
{
    aiString name;
    pAiMaterial->Get(AI_MATKEY_NAME, name);

    std::string nameStr = std::string(name.C_Str());
    if (nameStr.empty()) {
        Log::Warning("AssimpImporter: Material with no name found -> renaming to 'unnamed'.");
        nameStr = "unnamed";
    }

    std::unique_ptr<Material> pMaterial;
    if (importMode == ImportMode::GLTF2) pMaterial = std::make_unique<DefaultMaterial>();
    loadTextures(data, pAiMaterial, searchPath, pMaterial, importMode);

    float opacity = 1.0f;
    if(pAiMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
    {
        float diffuse = pMaterial->GetBaseDiffuse();
    }

    return std::move(pMaterial);
}
}   // namespace MapleLeaf