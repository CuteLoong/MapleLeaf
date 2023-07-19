#include "DeferredSubrender.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
static const uint32_t MAX_LIGHTS = 32;   // TODO: Make configurable.

DeferredSubrender::DeferredSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"E:/MapleLeaf/Resources/Shader/Deferred/Deferred.vert", "E:/MapleLeaf/Resources/Shader/Deferred/Deferred.frag"}, {}, {}, PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None)
{}

void DeferredSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();

    // To do light

    // Update uniforms
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPosition", camera->GetPosition());

    // Updates storage buffers.
    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("samplerPosition", Graphics::Get()->GetAttachment("position"));
    // descriptorSet.Push("samplerDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    // descriptorSet.Push("samplerNormal", Graphics::Get()->GetAttachment("normal"));
    // descriptorSet.Push("samplerMaterial", Graphics::Get()->GetAttachment("material"));

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the object.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}   // namespace MapleLeaf