#include "FrustumCullingSubrender.hpp"

#include "DescriptorHandler.hpp"
#include "PushHandler.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
FrustumCullingSubrender::FrustumCullingSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , compute("Shader/GPUDriven/Culling.comp")
    , pipeline(pipelineStage, {"Shader/GPUDriven/Default.vert", "Shader/GPUDriven/Default.frag"}, {Vertex3D::GetVertexInput()}, {},
               PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::ReadWrite, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
               VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false)
{}

void FrustumCullingSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;
    if (!gpuScene->GetIndirectBuffer()) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraCompute);

    uint32_t instanceCount = gpuScene->GetInstanceCount();

    pushHandler.Push("instanceCount", instanceCount);

    descriptorSetCompute.Push("InstanceDatas", gpuScene->GetInstanceDatasHandler());
    descriptorSetCompute.Push("DrawCommandBuffer", gpuScene->GetIndirectBuffer());
    descriptorSetCompute.Push("UniformCamera", uniformCameraCompute);
    descriptorSetCompute.Push("PushObject", pushHandler);

    if (!descriptorSetCompute.Update(compute)) return;
    compute.BindPipeline(commandBuffer);
    descriptorSetCompute.BindDescriptor(commandBuffer, compute);
    pushHandler.BindPush(commandBuffer, compute);
    compute.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));

    gpuScene->GetIndirectBuffer()->IndirectBufferPipelineBarrier(commandBuffer);
}

void FrustumCullingSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSetGraphics.Push("UniformCamera", uniformCamera);
    gpuScene->PushDescriptors(descriptorSetGraphics);

    if (!descriptorSetGraphics.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSetGraphics.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}

void FrustumCullingSubrender::PostRender(const CommandBuffer& commandBuffer) {}

}   // namespace MapleLeaf