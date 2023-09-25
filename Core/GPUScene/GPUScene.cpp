#include "GPUScene.hpp"
#include "Scenes.hpp"

#include "config.h"

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
#ifdef MAPLELEAF_GPUSCENE_DEBUG
    auto debugStart = Time::Now();
#endif
    // Now only instance alter, e.g. instance update but not add or remove
    // TODO UpdateMaterial and instance Add or Delete

    bool UpdateGPUScene = false;
    for (uint32_t i = 0; i < instances.size(); i++) {
        GPUInstance& instance = instances[i];
        instance.Update();

        if (instance.GetInstanceStatus() == GPUInstance::Status::ModelChanged || instance.GetInstanceStatus() == GPUInstance::Status::MatrixChanged) {
            instancesData[i] = InstanceData(instance.modelMatrix,
                                            instance.AABBLocalMin,
                                            instance.indexCount,
                                            instance.AABBLocalMax,
                                            instance.indexOffset,
                                            instance.vertexCount,
                                            instance.vertexOffset,
                                            instance.instanceID,
                                            instance.materialID);
        }

        UpdateGPUScene |= (instance.GetInstanceStatus() == GPUInstance::Status::ModelChanged);

        // VkDrawIndexedIndirectCommand indirectCmd = {};
        // indirectCmd.firstIndex                   = instance.indexOffset;
        // indirectCmd.firstInstance                = instance.instanceID;
        // indirectCmd.indexCount                   = instance.indexCount;
        // indirectCmd.instanceCount                = 1;
        // indirectCmd.vertexOffset                 = instance.vertexOffset;

        // drawCommands.push_back(indirectCmd);
    }

    if (UpdateGPUScene) {
        SetIndices(GPUInstance::indicesArray);
        SetVertices(GPUInstance::verticesArray);
    }

#ifdef MAPLELEAF_GPUSCENE_DEBUG
    Log::Out("Update Vertices costs: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    instancesHandler.Push(instancesData.data(), sizeof(InstanceData) * instancesData.size());
    materialsHandler.Push(materialsData.data(), sizeof(MaterialData) * materialsData.size());
#ifdef MAPLELEAF_GPUSCENE_DEBUG
    Log::Out("Update StorageBuffer Data costs: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif

    DrawCulling = Resources::Get()->GetThreadPool().Enqueue(ComputeFrustumCulling, instancesHandler.GetStorageBuffer());
}

void GPUScene::PushDescriptors(DescriptorsHandler& descriptorSet)
{
    descriptorSet.Push("InstanceDatas", instancesHandler);
    descriptorSet.Push("MaterialDatas", materialsHandler);
    descriptorSet.Push("DrawCommandBuffer", *DrawCulling);

    for (int i = 0; i < GPUMaterial::images.size(); i++) {
        descriptorSet.Push("ImageSamplers", GPUMaterial::images[i], i);
    }
}

bool GPUScene::cmdRender(const CommandBuffer& commandBuffer)
{
    if ((*DrawCulling) == nullptr) return false;
    uint32_t drawCount = (*DrawCulling)->GetSize() / sizeof(VkDrawIndexedIndirectCommand);
    if (vertexBuffer && indexBuffer) {
        VkBuffer     vertexBuffers[1] = {vertexBuffer->GetBuffer()};
        VkDeviceSize offsets[1]       = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(commandBuffer, (*DrawCulling)->GetBuffer(), 0, drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }
    else {
        return false;
    }

    return true;
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

std::unique_ptr<IndirectBuffer> GPUScene::ComputeFrustumCulling(const StorageBuffer* instanceBuffer)
{
    if (!instanceBuffer) return nullptr;
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    auto camera   = Scenes::Get()->GetScene()->GetCamera();

    uint32_t instanceCount = gpuScene->GetInstanceCount();

    std::unique_ptr<IndirectBuffer> indirectBuffer = std::make_unique<IndirectBuffer>(instanceCount * sizeof(VkDrawIndexedIndirectCommand));

    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("F:/MapleLeaf/Resources/Shader/GPUDriven/Culling.comp");

    compute.BindPipeline(commandBuffer);

    DescriptorsHandler descriptorSet(compute);
    PushHandler        pushHandler(*compute.GetShader()->GetUniformBlock("PushObject"));
    pushHandler.Push("projection", camera->GetProjectionMatrix());
    pushHandler.Push("view", camera->GetViewMatrix());
    pushHandler.Push("cameraPos", camera->GetPosition());
    pushHandler.Push("instanceCount", instanceCount);

    descriptorSet.Push("InstanceDatas", instanceBuffer);
    descriptorSet.Push("DrawCommandBuffer", indirectBuffer);
    descriptorSet.Push("PushObject", pushHandler);
    descriptorSet.Update(compute);

    descriptorSet.BindDescriptor(commandBuffer, compute);
    pushHandler.BindPush(commandBuffer, compute);
    compute.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));
    commandBuffer.SubmitIdle();

    return indirectBuffer;
}
}   // namespace MapleLeaf