#include "IndirectDrawPrevMV.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {

IndirectDrawPrevMV::IndirectDrawPrevMV(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/GPUDriven/DefaultPrev.vert", "Shader/GPUDriven/DefaultPrev.frag"}, {Vertex3D::GetVertexInput()}, {},
               PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::ReadWrite, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
               VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false)
{}

void IndirectDrawPrevMV::PreRender(const CommandBuffer& commandBuffer) {}

void IndirectDrawPrevMV::Render(const CommandBuffer& commandBuffer)
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

void IndirectDrawPrevMV::PostRender(const CommandBuffer& commandBuffer) {}

}   // namespace MapleLeaf