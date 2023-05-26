#pragma once

#include "NonCopyable.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/pbrmaterial.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <filesystem>
#include <unordered_map>

namespace MapleLeaf {
class ImporterData;
class SceneBuilder;

enum class ImportMode
{
    Default,
    OBJ,
    GLTF2,
};

template<typename T>
class AssimpImporter : public NonCopyable
{
public:
    class ImporterData
    {
    public:
        std::filesystem::path path;
        const aiScene*        pScene;
        SceneBuilder&         builder;

        std::unordered_map<uint32_t, std::unique_ptr<T>> materialMap;

        ImporterData(const std::filesystem::path& path, const aiScene* pAiScene, SceneBuilder& builder)
            : path(path)
            , pScene(pAiScene)
            , builder(builder)
        {}

        void AddAiNode(const aiNode* pNode)
        {
            if (mAiNodes.find(pNode->mName.C_Str()) == mAiNodes.end()) {
                mAiNodes[pNode->mName.C_Str()] = {};
            }
            mAiNodes[pNode->mName.C_Str()].push_back(pNode);
        }
        uint32_t GetNodeInstanceCount(const std::string& nodeName) const { return (uint32_t)mAiNodes.at(nodeName).size(); }

    private:
        std::unordered_map<const std::string, std::vector<const aiNode*>> mAiNodes;
    };

    AssimpImporter() = default;

    void Import(const std::filesystem::path& path, SceneBuilder& builder);

private:
    void LoadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath, std::unique_ptr<T>& pMaterial,
                      ImportMode importMode);
    void CreateAllMaterials(ImporterData& data, const std::filesystem::path& searchPath, ImportMode importMode);
    std::unique_ptr<T> CreateMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                      ImportMode importMode);

    void CreateSceneGraph(ImporterData& data);
    void ParseNode(ImporterData& data, const aiNode* pCurrent, bool hasBoneAncestor);

    void CreateMeshes(ImporterData& data);
};
}   // namespace MapleLeaf