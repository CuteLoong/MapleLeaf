#include "StereoMaskSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
StereoMaskSubrender::StereoMaskSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/StereoMask/StereoMask.vert", "Shader/StereoMask/StereoMask.frag"}, {}, {},
                                PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None))
{}

void StereoMaskSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetStereoProjectionMatrix());
    uniformScene.Push("view", camera->GetStereoViewMatrix()); 
    uniformScene.Push("zBufferParams", camera->GetZBufferParams());

    descriptorSet.Push("UniformScene", uniformScene);
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