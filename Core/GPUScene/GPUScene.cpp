#include "GPUScene.hpp"
#include "Scenes.hpp"

// #define MAPLELEAF_DEBUG

namespace MapleLeaf {
GPUScene::GPUScene() {}

GPUScene::~GPUScene()
{
    // Static class variables must be freed manually
    GPUMaterial::materialArray.clear();
    GPUMaterial::images.clear();
    GPUInstance::indicesArray.clear();
    GPUInstance::verticesArray.clear();
    GPUInstance::modelOffset.clear();
}

void GPUScene::Start()
{
    auto meshes = Scenes::Get()->GetScene()->GetComponents<Mesh>();

    for (const auto& mesh : meshes) {
        const auto& material = mesh->GetMaterial();
        if (GPUMaterial::GetMaterialID(material)) continue;

        materials.push_back(GPUMaterial(material));
    }

    for (const auto& mesh : meshes) {
        const auto& material = mesh->GetMaterial();
        instances.push_back(GPUInstance(mesh, instances.size(), GPUMaterial::GetMaterialID(material).value()));
    }

    for (auto& instance : instances) {
        instancesData.push_back(InstanceData(instance.modelMatrix,
                                             instance.AABBLocalMin,
                                             instance.indexCount,
                                             instance.AABBLocalMax,
                                             instance.indexOffset,
                                             instance.vertexCount,
                                             instance.vertexOffset,
                                             instance.instanceID,
                                             instance.materialID));
    }

    for (auto& material : materials) {
        materialsData.push_back(
            MaterialData(material.baseColor, material.metalic, material.roughness, material.baseColorTex, material.normalTex, material.materialTex));
    }


    SetIndices(GPUInstance::indicesArray);
    SetVertices(GPUInstance::verticesArray);
}

void GPUScene::Update()
{
#ifdef MAPLELEAF_DEBUG
    auto debugStart = Time::Now();
#endif
    // TODO UpdateMaterial

    bool UpdateGPUScene = false;
    drawCommands.clear();
    for (auto& instance : instances) {
        instance.Update();
        UpdateGPUScene |= (instance.GetInstanceStatus() == GPUInstance::Status::Changed);

        VkDrawIndexedIndirectCommand indirectCmd = {};
        indirectCmd.firstIndex                   = instance.indexOffset;
        indirectCmd.firstInstance                = instance.instanceID;
        indirectCmd.indexCount                   = instance.indexCount;
        indirectCmd.instanceCount                = 1;
        indirectCmd.vertexOffset                 = instance.vertexOffset;

        drawCommands.push_back(indirectCmd);
    }

    if (UpdateGPUScene) {
        SetIndices(GPUInstance::indicesArray);
        SetVertices(GPUInstance::verticesArray);
    }

#ifdef MAPLELEAF_DEBUG
    Log::Out("Update Vertices costs: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif
}

bool GPUScene::cmdRender(const CommandBuffer& commandBuffer, UniformHandler& uniformScene, PipelineGraphics& pipeline)
{
    pipeline.BindPipeline(commandBuffer);

    drawCommandBufferHandler.Push(drawCommands.data(), sizeof(VkDrawIndexedIndirectCommand) * drawCommands.size());
    instancesHandler.Push(instancesData.data(), sizeof(InstanceData) * instancesData.size());
    materialsHandler.Push(materialsData.data(), sizeof(MaterialData) * materialsData.size());

    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("DrawCommandBuffer", drawCommandBufferHandler);
    descriptorSet.Push("InstanceDatas", instancesHandler);
    descriptorSet.Push("MaterialDatas", materialsHandler);

    for (int i = 0; i < GPUMaterial::images.size(); i++) {
        descriptorSet.Push("ImageSamplers", GPUMaterial::images[i], i);
    }

    if (!descriptorSet.Update(pipeline)) return false;

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    if (vertexBuffer && indexBuffer) {
        VkBuffer     vertexBuffers[1] = {vertexBuffer->GetBuffer()};
        VkDeviceSize offsets[1]       = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(
            commandBuffer, drawCommandBufferHandler.GetIndirectBuffer()->GetBuffer(), 0, drawCommands.size(), sizeof(VkDrawIndexedIndirectCommand));
    }
    else {
        return false;
    }

    return true;
    // drawCommandBufferHandler.Push(drawCommands.data(), sizeof(VkDrawIndirectCommand) * drawCommands.size());
    // uint32_t drawCount = drawCommands.size();

    // drawCountHandler.Push(&drawCount, sizeof(uint32_t));
    // descriptorSet.Push("UniformScene", uniformScene);
    // descriptorSet.Push("DrawCommandBuffer", drawCommandBufferHandler);
    // descriptorSet.Push("DrawCount", drawCountHandler);

    // if (!descriptorSet.Update(pipeline)) return false;

    // descriptorSet.BindDescriptor(commandBuffer, pipeline);
}

void GPUScene::SetVertices(const std::vector<Vertex3D>& vertices)
{
    vertexBuffer = nullptr;

    if (vertices.empty()) return;

    Buffer vertexStaging(sizeof(Vertex3D) * vertices.size(),
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertices.data());
    vertexBuffer = std::make_unique<Buffer>(vertexStaging.GetSize(),
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = vertexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, vertexStaging.GetBuffer(), vertexBuffer->GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();
}

void GPUScene::SetIndices(const std::vector<uint32_t>& indices)
{
    indexBuffer = nullptr;

    if (indices.empty()) return;

    Buffer indexStaging(sizeof(uint32_t) * indices.size(),
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        indices.data());
    indexBuffer = std::make_unique<Buffer>(indexStaging.GetSize(),
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = indexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, indexStaging.GetBuffer(), indexBuffer->GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();
}
}   // namespace MapleLeaf