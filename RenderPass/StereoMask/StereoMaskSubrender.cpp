#include "StereoMaskSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
StereoMaskSubrender::StereoMaskSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/StereoMask/StereoMask.vert", "Shader/StereoMask/StereoMask.frag"}, {}, {},
                                PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::None))
{}

void StereoMaskSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void StereoMaskSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the quad.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void StereoMaskSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf