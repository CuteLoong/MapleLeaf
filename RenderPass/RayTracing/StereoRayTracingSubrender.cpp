#include "StereoRayTracingSubrender.hpp"

#include "Light.hpp"
#include "Scenes.hpp"
#include "Skybox.hpp"

namespace MapleLeaf {
StereoRayTracingSubrender::StereoRayTracingSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineRayTracing({"RayTracing/StereoRayTrace.rgen", "RayTracing/Raytrace.rmiss", "RayTracing/Raytrace.rchit"})
{}

void StereoRayTracingSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    const auto& skybox = Scenes::Get()->GetScene()->GetComponent<Skybox>();

    const auto& AS = Scenes::Get()->GetScene()->GetAsScene()->GetTopLevelAccelerationStruct();

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

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

    storagePointLights.Push(pointLights.data(), sizeof(PointLight) * pointLights.size());
    storageDirectionalLights.Push(directionalLights.data(), sizeof(DirectionalLight) * directionalLights.size());

    uniformSceneData.Push("vertexAddress", gpuScene->GetVertexBuffer()->GetDeviceAddress());
    uniformSceneData.Push("indexAddress", gpuScene->GetIndexBuffer()->GetDeviceAddress());
    uniformSceneData.Push("pointLightsCount", pointLights.size() - 1);
    uniformSceneData.Push("directionalLightsCount", directionalLights.size() - 1);

    uniformFrameData.Push("spp", 100);
    uniformFrameData.Push("maxDepth", 2);

    descriptorSet.Push("topLevelAS", AS);
    descriptorSet.Push("UniformFrameData", uniformFrameData);
    descriptorSet.Push("UniformSceneData", uniformSceneData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("BufferPointLights", storagePointLights);
    descriptorSet.Push("BufferDirectionalLights", storageDirectionalLights);
    if (skybox) descriptorSet.Push("SkyboxCubeMap", skybox->GetImageCube());
    gpuScene->PushDescriptors(descriptorSet);

    descriptorSet.Push("image", Graphics::Get()->GetNonRTAttachment("RayTracingTarget"));

    if (!descriptorSet.Update(pipelineRayTracing)) return;

    pipelineRayTracing.BindPipeline(commandBuffer);
    descriptorSet.BindDescriptor(commandBuffer, pipelineRayTracing);
    pipelineRayTracing.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void StereoRayTracingSubrender::Render(const CommandBuffer& commandBuffer) {}

void StereoRayTracingSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf