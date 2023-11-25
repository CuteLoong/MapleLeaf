#include "ToneMapingSubrender.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
ToneMapingSubrender::ToneMapingSubrender(const Pipeline::Stage& pipelineStage, ToneMappingInfo toneMappingInfo)
    : Subrender(pipelineStage)
    , toneMappingInfo(toneMappingInfo)
    , pipeline(pipelineStage, {"Shader/Deferred/ToneMapping.vert", "Shader/Deferred/ToneMapping.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
               PipelineGraphics::Depth::None)
{}

void ToneMapingSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void ToneMapingSubrender::Render(const CommandBuffer& commandBuffer)
{
    // Updates storage buffers.
    uniformToneMaping.Push("exposure", toneMappingInfo.exposure);
    uniformToneMaping.Push("gamma", toneMappingInfo.gamma);

    descriptorSet.Push("UniformToneMapping", uniformToneMaping);
    descriptorSet.Push("ResolvedImage", Graphics::Get()->GetAttachment("resolved"));

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void ToneMapingSubrender::PostRender(const CommandBuffer& commandBuffer) {}

}   // namespace MapleLeaf