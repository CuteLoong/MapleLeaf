#include "RayTracingToneMappingSubrender.hpp"
#include "Scenes.hpp"


namespace MapleLeaf {
RayTracingToneMappingSubrender::RayTracingToneMappingSubrender(const Pipeline::Stage& pipelineStage, ToneMappingInfo toneMappingInfo)
    : Subrender(pipelineStage)
    , toneMappingInfo(toneMappingInfo)
    , pipeline(pipelineStage, {"Shader/Deferred/RayTracingToneMapping.vert", "Shader/Deferred/RayTracingToneMapping.frag"}, {}, {},
               PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None)
{}

void RayTracingToneMappingSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void RayTracingToneMappingSubrender::Render(const CommandBuffer& commandBuffer)
{
    // Updates storage buffers.
    uniformToneMaping.Push("exposure", toneMappingInfo.exposure);
    uniformToneMaping.Push("gamma", toneMappingInfo.gamma);

    descriptorSet.Push("UniformToneMapping", uniformToneMaping);
    descriptorSet.Push("RayTracingTarget", Graphics::Get()->GetNonRTAttachment("RayTracingTarget"));

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void RayTracingToneMappingSubrender::PostRender(const CommandBuffer& commandBuffer) {}

}   // namespace MapleLeaf