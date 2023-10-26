#include "AssimpImporter.hpp"
#include "Color.hpp"
#include "DefaultMaterial.hpp"
#include "Devices.hpp"
#include "Files.hpp"
#include "Light.hpp"
#include "SceneGraph.hpp"
#include "Transform.hpp"

#include "config.h"

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

glm::mat4 AiCast(const aiMatrix4x4& aiMat)
{
    glm::mat4 ret{aiMat.a1,
                  aiMat.b1,
                  aiMat.c1,
                  aiMat.d1,
                  aiMat.a2,
                  aiMat.b2,
                  aiMat.c2,
                  aiMat.d2,
                  aiMat.a3,
                  aiMat.b3,
                  aiMat.c3,
                  aiMat.d3,
                  aiMat.a4,
                  aiMat.b4,
                  aiMat.c4,
                  aiMat.d4};
    return ret;
}

glm::vec3 AiCast(const aiVector3D& aiVec)
{
    glm::vec3 ret(aiVec.x, aiVec.y, aiVec.z);

    return ret;
}

template<typename T>
void AssimpImporter<T>::Import(const std::filesystem::path& path, Builder& builder)
{
#ifdef MAPLELEAF_SCENE_DEBUG
    auto debugStart = Time::Now();
#endif
    auto exisitPath = Files::Get()->GetExistPath(path);
    if (!exisitPath) {
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
    assimpFlags &= ~(aiProcess_OptimizeMeshes);             // Avoid merging original meshes

    int removeFlags = aiComponent_COLORS;
    for (uint32_t uvLayer = 1; uvLayer < AI_MAX_NUMBER_OF_TEXTURECOORDS; uvLayer++) removeFlags |= aiComponent_TEXCOORDSn(uvLayer);
    removeFlags |= aiComponent_TANGENTS_AND_BITANGENTS;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);

    const aiScene* pScene = importer.ReadFile(exisitPath->string().c_str(), assimpFlags);
    if (!pScene) Log::Error("Failed to open scene: ", importer.GetErrorString());

    ImporterData data(exisitPath.value(), pScene, builder);

    auto searchPath = exisitPath->parent_path();

    ImportMode importMode = ImportMode::Default;
    if (exisitPath->extension() == ".obj") importMode = ImportMode::OBJ;
    if (exisitPath->extension() == ".gltf" || exisitPath->extension() == ".glb") importMode = ImportMode::GLTF2;

    CreateAllMaterials(data, searchPath, importMode);
#ifdef MAPLELEAF_SCENE_DEBUG
    Log::Out("Create materials cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateSceneGraph(data);
#ifdef MAPLELEAF_SCENE_DEBUG
    Log::Out("Create scene graph cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateMeshes(data);
#ifdef MAPLELEAF_SCENE_DEBUG
    Log::Out("Create meshes cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateCameras(data, importMode);
#ifdef MAPLELEAF_SCENE_DEBUG
    Log::Out("Create cameras cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    CreateLights(data);
#ifdef MAPLELEAF_SCENE_DEBUG
    Log::Out("Create lights cost: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif
}

template<typename T>
void AssimpImporter<T>::LoadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                     std::shared_ptr<T>& pMaterial, ImportMode importMode)
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
        data.materialMap[i]           = std::move(CreateMaterial(data, pAiMaterial, searchPath, importMode));
        // data.builder.AddMaterial(std::move(data.materialMap[i]));
    }
}

template<typename T>
std::shared_ptr<T> AssimpImporter<T>::CreateMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                                     ImportMode importMode)
{
    aiString name;
    pAiMaterial->Get(AI_MATKEY_NAME, name);

    std::string nameStr = std::string(name.C_Str());
    if (nameStr.empty()) {
        Log::Warning("AssimpImporter: Material with no name found -> renaming to 'unnamed'. \n");
        nameStr = "unnamed";
    }

    std::shared_ptr<T> pMaterial;
    if (importMode == ImportMode::GLTF2) pMaterial = std::make_shared<DefaultMaterial>(Color::White, nullptr, 0, 0);
    LoadTextures(data, pAiMaterial, searchPath, pMaterial, importMode);

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
            Color baseColor = Color(color.r, color.g, color.b, pMaterial->GetBaseDiffuse().a);
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

    return pMaterial;
}

template<typename T>
void AssimpImporter<T>::CreateSceneGraph(ImporterData& data)
{
    aiNode* pRoot = data.pScene->mRootNode;

    ParseNode(data, pRoot, false);
}

template<typename T>
void AssimpImporter<T>::ParseNode(ImporterData& data, const aiNode* pCurrent, bool hasBoneAncestor)
{
    SceneNode node;
    node.name = pCurrent->mName.C_Str();
    aiVector3D position, rotaion, scale;
    pCurrent->mTransformation.Decompose(scale, rotaion, position);
    node.transform = new Transform(AiCast(position), AiCast(rotaion), AiCast(scale));
    node.transform->GetWorldMatrix();
    node.parent = pCurrent->mParent ? data.getNodeID(pCurrent->mParent) : NodeID::Invalid();
    node.meshes.resize(pCurrent->mNumMeshes);
    if (!node.meshes.empty()) std::copy(pCurrent->mMeshes, pCurrent->mMeshes + pCurrent->mNumMeshes, node.meshes.data());

    data.AddAiNode(pCurrent, data.builder.AddSceneNode(std::move(node)));
    for (uint32_t i = 0; i < pCurrent->mNumChildren; i++) {
        ParseNode(data, pCurrent->mChildren[i], hasBoneAncestor);
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
            Log::Warning("AssimpImporter: Mesh ", pMesh->mName.C_Str(), " has no faces, ignoring.\n");
            continue;
        }
        if (pMesh->mFaces->mNumIndices != 3) {
            Log::Warning("AssimpImporter: Mesh ", pMesh->mName.C_Str(), " is not a triangle mesh, ignoring.\n");
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

        assert(indexBuffer.size() <= std::numeric_limits<uint32_t>::max());
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

        data.builder.AddMesh(std::move(std::make_shared<Model>(vertexBuffer, indexBuffer)), std::move(data.materialMap[pAiMesh->mMaterialIndex]));
    }
}

template<typename T>
void AssimpImporter<T>::CreateDirLight(ImporterData& data, const aiLight* pAiLight)
{
    auto light = std::make_unique<Light>(LightType::Directional);

    assert(pAiLight->mColorDiffuse == pAiLight->mColorSpecular);
    Color color = Color(pAiLight->mColorSpecular.r, pAiLight->mColorSpecular.g, pAiLight->mColorSpecular.b);
    light->SetColor(color);
    light->SetName(pAiLight->mName.C_Str());
    light->SetDirection(glm::normalize(AiCast(pAiLight->mDirection)));

    data.builder.AddLight(std::move(light));
}

template<typename T>
void AssimpImporter<T>::CreatePointLight(ImporterData& data, const aiLight* pAiLight)
{
    auto light = std::make_unique<Light>(LightType::Point);

    assert(pAiLight->mColorDiffuse == pAiLight->mColorSpecular);
    Color color = Color(pAiLight->mColorSpecular.r, pAiLight->mColorSpecular.g, pAiLight->mColorSpecular.b);
    light->SetColor(color);
    light->SetName(pAiLight->mName.C_Str());
    light->SetPosition(glm::normalize(AiCast(pAiLight->mPosition)));

    glm::vec3 attenuation = glm::vec3(pAiLight->mAttenuationConstant, pAiLight->mAttenuationLinear, pAiLight->mAttenuationQuadratic);
    light->SetAttenuation(attenuation);

    data.builder.AddLight(std::move(light));
}

template<typename T>
void AssimpImporter<T>::CreateLights(ImporterData& data)
{
    for (uint32_t i = 0; i < data.pScene->mNumLights; i++) {
        const aiLight* pAiLight = data.pScene->mLights[i];
        switch (pAiLight->mType) {
        case aiLightSource_UNDEFINED: Log::Info("The light undefined! \n"); break;
        case aiLightSource_DIRECTIONAL: CreateDirLight(data, pAiLight); break;
        case aiLightSource_POINT: CreatePointLight(data, pAiLight); break;
        case aiLightSource_SPOT: break;
        case aiLightSource_AMBIENT: break;
        case aiLightSource_AREA: break;
        case _aiLightSource_Force32Bit: break;
        }
    }
}

template<typename T>
void AssimpImporter<T>::CreateCameras(ImporterData& data, ImportMode importMode)
{
    for (uint32_t i = 0; i < data.pScene->mNumCameras; i++) {
        const aiCamera* pAiCamera = data.pScene->mCameras[i];
        auto            camera    = std::make_unique<Camera>();

        camera->SetName(pAiCamera->mName.C_Str());
        camera->SetPosition(AiCast(pAiCamera->mPosition));
        camera->SetUpVector(AiCast(pAiCamera->mUp));

        float aspectRatio = pAiCamera->mAspect != 0.f ? pAiCamera->mAspect : Devices::Get()->GetWindow()->GetAspectRatio();
        float fieldOfView = importMode == ImportMode::GLTF2 ? 2 * atan(tan(pAiCamera->mHorizontalFOV / 2) / aspectRatio) : glm::radians(30.0f);
        camera->SetFieldOfView(fieldOfView);
        camera->SetAspectRatio(aspectRatio);
        camera->SetNearPlane(pAiCamera->mClipPlaneNear);
        camera->SetFarPlane(pAiCamera->mClipPlaneFar);

        data.builder.AddCamera(std::move(camera));
    }
}

template class AssimpImporter<DefaultMaterial>;
}   // namespace MapleLeaf