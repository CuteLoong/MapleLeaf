#include "IndirectDrawStereoSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawStereoSubrender::IndirectDrawStereoSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , compute("Shader/GPUDriven/CullingStereo.comp")
    , pipeline(pipelineStage, {"Shader/GPUDriven/DefaultStereo.vert", "Shader/GPUDriven/Multiview.geom", "Shader/GPUDriven/DefaultStereo.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::StereoMRT)
{}

void IndirectDrawStereoSubrender::PreRender(const CommandBuffer& commandBuffer)
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

void IndirectDrawStereoSubrender::Render(const CommandBuffer& commandBuffer)
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

void IndirectDrawStereoSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf