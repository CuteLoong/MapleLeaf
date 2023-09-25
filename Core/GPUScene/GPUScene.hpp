#pragma once

#include "DescriptorHandler.hpp"
#include "GPUInstance.hpp"
#include "GPUMaterial.hpp"
#include "IndirectHandler.hpp"
#include "StorageHandler.hpp"

namespace MapleLeaf {
class GPUScene
{
    friend class Scene;

public:
    GPUScene();

    ~GPUScene();

    void Start();
    void Update();

    void SetVertices(const std::vector<Vertex3D>& vertices);
    void SetIndices(const std::vector<uint32_t>& indices);

    void PushDescriptors(DescriptorsHandler& descriptorSet);
    bool cmdRender(const CommandBuffer& commandBuffer, std::unique_ptr<IndirectBuffer>& indirectBuffer);

    const StorageHandler& GetInstanceDatasHandler() const { return instancesHandler; }

    uint32_t GetInstanceCount() const { return instances.size(); }

private:
    struct InstanceData
    {
        glm::mat4 modelMatrix;
        glm::vec3 AABBLocalMin;
        uint32_t  indexCount;
        glm::vec3 AABBLocalMax;
        uint32_t  indexOffset;
        uint32_t  vertexCount;
        uint32_t  vertexOffset;
        uint32_t  instanceID;
        uint32_t  materialID;

        InstanceData(glm::mat4 modelMatrix, glm::vec3 AABBLocalMin, uint32_t indexCount, glm::vec3 AABBLocalMax, uint32_t indexOffset,
                     uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceID, uint32_t materialID)
            : modelMatrix(modelMatrix)
            , AABBLocalMin(AABBLocalMin)
            , indexCount(indexCount)
            , AABBLocalMax(AABBLocalMax)
            , indexOffset(indexOffset)
            , vertexCount(vertexCount)
            , vertexOffset(vertexOffset)
            , instanceID(instanceID)
            , materialID(materialID)
        {}
    };

    struct MaterialData
    {
        Color   baseColor;
        float   metalic;
        float   roughness;
        int32_t baseColorTex;
        int32_t normalTex;
        int32_t materialTex;
        int32_t padding1;
        int32_t padding2;
        int32_t padding3;

        MaterialData(Color baseColor, float metalic, float roughness, int32_t baseColorTex, int32_t normalTex, int32_t materialTex)
            : baseColor(baseColor)
            , metalic(metalic)
            , roughness(roughness)
            , baseColorTex(baseColorTex)
            , normalTex(normalTex)
            , materialTex(materialTex)
        {}
    };

    bool started = false;

    std::vector<GPUInstance> instances;
    std::vector<GPUMaterial> materials;

    std::unique_ptr<Buffer>   vertexBuffer;
    std::unique_ptr<Buffer>   indexBuffer;
    std::vector<InstanceData> instancesData;
    std::vector<MaterialData> materialsData;

    StorageHandler     instancesHandler;
    StorageHandler     materialsHandler;
    DescriptorsHandler descriptorSet;
};
}   // namespace MapleLeaf