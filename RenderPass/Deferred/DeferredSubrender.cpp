#include "DeferredSubrender.hpp"
#include "Light.hpp"
#include "Scenes.hpp"
#include "ShadowSystem.hpp"


namespace MapleLeaf {
// static const uint32_t MAX_LIGHTS = 32;   // TODO: Make configurable.

DeferredSubrender::DeferredSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/Deferred/Deferred.vert", "Shader/Deferred/Deferred.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
               PipelineGraphics::Depth::None)
{}

void DeferredSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void DeferredSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();

    auto                          sceneLights = Scenes::Get()->GetScene()->GetComponents<Light>();
    std::vector<PointLight>       pointLights(1, PointLight());
    std::vector<DirectionalLight> directionalLights(1, DirectionalLight());

    for (const auto& light : sceneLights) {
        if (light->type == LightType::Directional) {
            DirectionalLight directionalLight = {};
            directionalLight.color            = light->GetColor();
            directionalLight.direction        = light->GetDirection();
            directionalLights.push_back(directionalLight);
        }
        else if (light->type == LightType::Point) {
            PointLight pointLight = {};
            pointLight.color      = light->GetColor();
            if (auto transform = light->GetEntity()->GetComponent<Transform>()) {
                pointLight.position = transform->GetPosition();
            }
            pointLight.attenuation = light->GetAttenuation();
            pointLights.push_back(pointLight);
        }
    }

    // Update uniforms
    uniformScene.Push("view", camera->GetViewMatrix());
    if (auto shadows = Scenes::Get()->GetScene()->GetSystem<ShadowSystem>())
        uniformScene.Push("shadowMatrix", shadows->GetShadowCascade().GetLightProjectionViewMatrix());
    uniformScene.Push("cameraPosition", camera->GetPosition());
    uniformScene.Push("pointLightsCount", pointLights.size() - 1);
    uniformScene.Push("directionalLightsCount", directionalLights.size() - 1);

    // Update Light buffer
    storagePointLights.Push(pointLights.data(), sizeof(PointLight) * pointLights.size());
    storageDirectionalLights.Push(directionalLights.data(), sizeof(DirectionalLight) * directionalLights.size());

    // Updates storage buffers.
    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("BufferPointLights", storagePointLights);
    descriptorSet.Push("BufferDirectionalLights", storageDirectionalLights);

    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));
    descriptorSet.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));

    descriptorSet.Push("inShadowMap", Graphics::Get()->GetAttachment("shadows"));

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the object.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void DeferredSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf