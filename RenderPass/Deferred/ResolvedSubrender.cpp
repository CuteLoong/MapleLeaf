#include "ResolvedSubrender.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
ResolvedSubrender::ResolvedSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/Deferred/Resolved.vert", "Shader/Deferred/Resolved.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
               PipelineGraphics::Depth::None)
{}

void ResolvedSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void ResolvedSubrender::Render(const CommandBuffer& commandBuffer)
{
    // Updates storage buffers.
    descriptorSet.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));
    descriptorSet.Push("SSRHitsMap", Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void ResolvedSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf