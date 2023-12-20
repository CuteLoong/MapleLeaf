#include "RayTracingSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
RayTracingSubrender::RayTracingSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineRayTracing({"RayTracing/Raytrace.rgen", "RayTracing/Raytrace.rmiss", "RayTracing/Raytrace.rchit"})
    , sceneDescription{}
{}

void RayTracingSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    sceneDescription.vertexAddress = gpuScene->GetVertexBuffer()->GetDeviceAddress();
    sceneDescription.indexAddress  = gpuScene->GetIndexBuffer()->GetDeviceAddress();

    const auto& AS = Scenes::Get()->GetScene()->GetAsScene()->GetTopLevelAccelerationStruct();

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformSceneData.Push("vertexAddress", sceneDescription.vertexAddress);
    uniformSceneData.Push("indexAddress", sceneDescription.indexAddress);

    descriptorSet.Push("topLevelAS", AS);
    descriptorSet.Push("UniformSceneData", uniformSceneData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    // descriptorSet.Push("InstanceDatas", gpuScene->GetInstanceDatasHandler());
    // descriptorSet.Push("MaterialDatas", gpuScene->GetMaterialDatasHandler());
    // for (int i = 0; i < GPUMaterial::images.size(); i++) {
    //     descriptorSet.Push("ImageSamplers", GPUMaterial::images[i], i);
    // }
    gpuScene->PushDescriptors(descriptorSet);

    descriptorSet.Push("image", Graphics::Get()->GetNonRTAttachment("RayTracingTarget"));

    if (!descriptorSet.Update(pipelineRayTracing)) return;

    pipelineRayTracing.BindPipeline(commandBuffer);
    descriptorSet.BindDescriptor(commandBuffer, pipelineRayTracing);
    pipelineRayTracing.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void RayTracingSubrender::Render(const CommandBuffer& commandBuffer) {}

void RayTracingSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf