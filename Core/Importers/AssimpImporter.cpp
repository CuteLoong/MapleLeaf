#include "AssimpImporter.hpp"
#include "Color.hpp"
#include "DefaultMaterial.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "SceneBuilder.hpp"
#include "Vertex.hpp"
#include "assimp/types.h"
#include <memory>
#include <utility>
#include <vector>


namespace MapleLeaf {
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

template<typename T>
void AssimpImporter<T>::Import(const std::filesystem::path& path, SceneBuilder& builder)
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

    CreateAllMaterials(data, searchPath, importMode);
#ifdef MAPLELEAF_DEBUG
    Log::Out("Create materials cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateSceneGraph(data);
#ifdef MAPLELEAF_DEBUG
    Log::Out("Create scene graph cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateMeshes(data);
    // TODO instances
#ifdef MAPLELEAF_DEBUG
    Log::Out("Create meshes cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif
}

template<typename T>
void AssimpImporter<T>::LoadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                     std::unique_ptr<T>& pMaterial, ImportMode importMode)
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

template<typename T>
void AssimpImporter<T>::CreateAllMaterials(ImporterData& data, const std::filesystem::path& searchPath, ImportMode importMode)
{
    for (uint32_t i = 0; i < data.pScene->mNumMaterials; i++) {
        const aiMaterial* pAiMaterial = data.pScene->mMaterials[i];
        data.materialMap[i]           = std::move(createMaterial(data, pAiMaterial, searchPath, importMode));
    }
}

template<typename T>
std::unique_ptr<T> AssimpImporter<T>::CreateMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                                     ImportMode importMode)
{
    aiString name;
    pAiMaterial->Get(AI_MATKEY_NAME, name);

    std::string nameStr = std::string(name.C_Str());
    if (nameStr.empty()) {
        Log::Warning("AssimpImporter: Material with no name found -> renaming to 'unnamed'.");
        nameStr = "unnamed";
    }

    std::unique_ptr<T> pMaterial;
    if (importMode == ImportMode::GLTF2) pMaterial = std::make_unique<DefaultMaterial>();
    loadTextures(data, pAiMaterial, searchPath, pMaterial, importMode);

    float opacity = 1.0f;
    if (pAiMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        Color diffuse = pMaterial->GetBaseDiffuse();
        diffuse.a     = opacity;
        pMaterial->SetBaseDiffuse(diffuse);
    }

    // Diffuse color
    aiColor3D color;
    if (pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        Color diffuse(color.r, color.g, color.b, pMaterial->GetBaseDiffuse().a);
        pMaterial->SetBaseDiffuse(diffuse);
    }

    if (importMode == ImportMode::GLTF2) {
        if (pAiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, color) == AI_SUCCESS) {
            Color baseColor = float4(color.r, color.g, color.b, pMaterial->getBaseColor().a);
            pMaterial->SetBaseDiffuse(baseColor);
        }

        float metallic;
        if (pAiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
            pMaterial->SetMetallic(metallic);
        }

        float roughness;
        if (pAiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            pMaterial->SetRoughness(roughness);
        }
    }

    return std::move(pMaterial);
}

template<typename T>
void AssimpImporter<T>::CreateSceneGraph(ImporterData& data)
{
    // TODO BoneList
    aiNode* pRoot = data.pScene->mRootNode;
    parseNode(data, pRoot, false);
}

template<typename T>
void AssimpImporter<T>::ParseNode(ImporterData& data, const aiNode* pCurrent, bool hasBoneAncestor)
{
    // TODO: store a tree in scene for bone
    data.AddAiNode(pCurrent);
    for (uint32_t i = 0; i < pCurrent->mNumChildren; i++) {
        parseNode(data, pCurrent->mChildren[i], hasBoneAncestor);
    }
}

template<typename T>
void AssimpImporter<T>::CreateMeshes(ImporterData& data)
{
    const aiScene* pScene = data.pScene;

    std::vector<const aiMesh*> meshes;
    for (uint32_t i = 0; i < pScene->mNumMeshes; i++) {
        const aiMesh* pMesh = pScene->mMeshes[i];
        if (!pMesh->HasFaces()) {
            Log::Warning("AssimpImporter: Mesh ", pMesh->mName.C_Str(), " has no faces, ignoring.");
            continue;
        }
        if (pMesh->mFaces->mNumIndices != 3) {
            Log::Warning("AssimpImporter: Mesh ", pMesh->mName.C_Str(), " is not a triangle mesh, ignoring.");
            continue;
        }
        meshes.push_back(pMesh);
    }

    for (const auto& pAiMesh : meshes) {
        std::vector<uint32_t> indexBuffer;
        std::vector<Vertex3D> vertexBuffer;

        // index buffer read
        const uint32_t perFaceIndexCount = pAiMesh->mFaces[0].mNumIndices;
        const uint32_t indexCount        = pAiMesh->mNumFaces * perFaceIndexCount;
        indexBuffer.resize(indexCount);

        for (uint32_t i = 0; i < pAiMesh->mNumFaces; i++) {
            assert(pAiMesh->mFaces[i].mNumIndices == perFaceIndexCount);
            for (uint32_t j = 0; j < perFaceIndexCount; j++) indexBuffer[i * perFaceIndexCount + j] = (uint32_t)(pAiMesh->mFaces[i].mIndices[j]);
        }

        assert(indexList.size() <= std::numeric_limits<uint32_t>::max());
        // vertex buffer read
        assert(pAiMesh->mVertices);
        vertexBuffer.resize(pAiMesh->mNumVertices);
        static_assert(sizeof(pAiMesh->mVertices[0]) == sizeof(vertexBuffer[0].position));
        static_assert(sizeof(pAiMesh->mNormals[0]) == sizeof(vertexBuffer[0].normal));

        for (uint32_t i = 0; i < pAiMesh->mNumVertices; i++) {
            glm::vec3 position = glm::vec3(pAiMesh->mVertices[i].x, pAiMesh->mVertices[i].y, pAiMesh->mVertices[i].z);
            glm::vec3 normal   = glm::vec3(pAiMesh->mNormals[i].x, pAiMesh->mNormals[i].y, pAiMesh->mNormals[i].z);
            glm::vec2 uv;
            if (pAiMesh->HasTextureCoords(0)) uv = glm::vec2(pAiMesh->mTextureCoords[0][i].x, pAiMesh->mTextureCoords[0][i].y);

            vertexBuffer[i] = std::move(Vertex3D(position, uv, normal));
        }

        data.builder.AddModel(pAiMesh->mName, std::move(std::make_shared<Model>(vertexBuffer, indexBuffer)));
        data.builder.materialIds.push_back(pAiMesh->mMaterialIndex);
    }
}
}   // namespace MapleLeaf