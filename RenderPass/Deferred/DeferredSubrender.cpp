#include "DeferredSubrender.hpp"
#include "Light.hpp"
#include "Scenes.hpp"


namespace MapleLeaf {
// static const uint32_t MAX_LIGHTS = 32;   // TODO: Make configurable.

DeferredSubrender::DeferredSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"E:/MapleLeaf/Resources/Shader/Deferred/Deferred.vert", "E:/MapleLeaf/Resources/Shader/Deferred/Deferred.frag"}, {},
               {}, PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None)
{}

void DeferredSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();

    // To do light
    auto                    sceneLights = Scenes::Get()->GetScene()->GetComponents<Light>();
    uint32_t                lightCount  = sceneLights.size();
    std::vector<PointLight> pointLights;

    for (const auto& light : sceneLights) {
        PointLight pointLight = {};
        pointLight.color = light->GetColor();
        pointLight.position = light->GetPosition();
        pointLight.attenuation = light->GetAttenuation();
        pointLights.push_back(pointLight);
    }

    // Update uniforms
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPosition", camera->GetPosition());
    uniformScene.Push("lightsCount", lightCount);

    // Update Light buffer
    storageLights.Push(pointLights.data(), sizeof(PointLight) * lightCount);

    // Updates storage buffers.
    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("BufferLights", storageLights);
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));
    descriptorSet.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the object.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}   // namespace MapleLeaf