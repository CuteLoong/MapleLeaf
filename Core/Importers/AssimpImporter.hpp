#pragma once

#include "DefaultBuilder.hpp"
#include "DefaultMaterial.hpp"
#include "NonCopyable.hpp"
#include "SceneGraph.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/pbrmaterial.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <filesystem>
#include <map>

namespace MapleLeaf {
class ImporterData;
class Builder;

enum class ImportMode
{
    Default,
    OBJ,
    GLTF2,
};

template<typename T>
class AssimpImporter
{
public:
    class ImporterData
    {
    public:
        ImporterData() = default;

        std::filesystem::path path;
        const aiScene*        pScene;
        Builder&              builder;

        std::map<uint32_t, std::shared_ptr<T>> materialMap;

        ImporterData(const std::filesystem::path& path, const aiScene* pAiScene, Builder& builder)
            : path(path)
            , pScene(pAiScene)
            , builder(builder)
        {}

        NodeID getNodeID(const aiNode* pNode) const { return mAiToNodeID.at(pNode); }
        NodeID getNodeID(const std::string name, uint32_t index) const
        {
            try {
                return getNodeID(mAiNodes.at(name)[index]);
            }
            catch (const std::exception&) {
                return NodeID::Invalid();
            }
        }

        void AddAiNode(const aiNode* pNode, NodeID nodeID)
        {
            assert(mAiToNodeID.find(pNode) == mAiToNodeID.end());
            mAiToNodeID[pNode] = nodeID;

            if (mAiNodes.find(pNode->mName.C_Str()) == mAiNodes.end()) {
                mAiNodes[pNode->mName.C_Str()] = {};
            }
            mAiNodes[pNode->mName.C_Str()].push_back(pNode);
        }
        uint32_t GetNodeInstanceCount(const std::string& nodeName) const { return (uint32_t)mAiNodes.at(nodeName).size(); }

    private:
        std::map<const std::string, std::vector<const aiNode*>> mAiNodes;
        std::map<const aiNode*, NodeID>                         mAiToNodeID;
    };

    AssimpImporter() = default;

    void Import(const std::filesystem::path& path, Builder& builder);

private:
    void LoadTextures(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath, std::shared_ptr<T>& pMaterial,
                      ImportMode importMode);
    void CreateAllMaterials(ImporterData& data, const std::filesystem::path& searchPath, ImportMode importMode);
    std::shared_ptr<T> CreateMaterial(ImporterData& data, const aiMaterial* pAiMaterial, const std::filesystem::path& searchPath,
                                      ImportMode importMode);

    void CreateSceneGraph(ImporterData& data);
    void ParseNode(ImporterData& data, const aiNode* pCurrent, bool hasBoneAncestor);
    void ParseAnimation(ImporterData& data, const aiAnimation* pAiAnimation, ImportMode importMode);

    void CreateMeshes(ImporterData& data);

    void CreateLights(ImporterData& data);
    void CreateCameras(ImporterData& data, ImportMode importMode);
    void CreateDirLight(ImporterData& data, const aiLight* pAiLight);
    void CreatePointLight(ImporterData& data, const aiLight* pAiLight);
    void CreateAnimations(ImporterData& data, ImportMode importMode);
};
}   // namespace MapleLeaf