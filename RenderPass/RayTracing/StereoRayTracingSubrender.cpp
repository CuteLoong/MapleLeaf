#include "StereoRayTracingSubrender.hpp"

#include "Light.hpp"
#include "Scenes.hpp"
#include "Skybox.hpp"

namespace MapleLeaf {
StereoRayTracingSubrender::StereoRayTracingSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineRayTracing({"RayTracing/StereoRayTrace.rgen", "RayTracing/Raytrace.rmiss", "RayTracing/Raytrace.rchit"})
    , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void StereoRayTracingSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    const auto& skybox = Scenes::Get()->GetScene()->GetComponent<Skybox>();

    if (this->skybox != skybox->GetImageCube()) {
        this->skybox = skybox->GetImageCube();
        irradiance   = Resources::Get()->GetThreadPool().Enqueue(ComputeIrradiance, skybox->GetImageCube(), 64);
        prefiltered  = Resources::Get()->GetThreadPool().Enqueue(ComputePrefiltered, skybox->GetImageCube(), 512);
    }

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

    uniformFrameData.Push("spp", 128);
    uniformFrameData.Push("maxDepth", 2);

    descriptorSet.Push("topLevelAS", AS);
    descriptorSet.Push("UniformFrameData", uniformFrameData);
    descriptorSet.Push("UniformSceneData", uniformSceneData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("BufferPointLights", storagePointLights);
    descriptorSet.Push("BufferDirectionalLights", storageDirectionalLights);
    if (skybox) {
        descriptorSet.Push("SkyboxCubeMap", skybox->GetImageCube());
        descriptorSet.Push("samplerBRDF", *brdf);
        descriptorSet.Push("samplerIrradiance", *irradiance);
        descriptorSet.Push("samplerPrefiltered", *prefiltered);
    }
    gpuScene->PushDescriptors(descriptorSet);

    descriptorSet.Push("image", Graphics::Get()->GetNonRTAttachment("RayTracingTarget"));
    descriptorSet.Push("indirectLightResult", Graphics::Get()->GetNonRTAttachment("RayTracingIndirect"));

    if (!descriptorSet.Update(pipelineRayTracing)) return;

    pipelineRayTracing.BindPipeline(commandBuffer);
    descriptorSet.BindDescriptor(commandBuffer, pipelineRayTracing);
    pipelineRayTracing.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void StereoRayTracingSubrender::Render(const CommandBuffer& commandBuffer) {}

void StereoRayTracingSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::unique_ptr<Image2d> StereoRayTracingSubrender::ComputeBRDF(uint32_t size)
{
    auto brdfImage = std::make_unique<Image2d>(glm::uvec2(size), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);

    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("Shader/Skybox/PreIntegrationDFG.comp");

    // Bind the pipeline.
    compute.BindPipeline(commandBuffer);

    // Updates descriptors.
    DescriptorsHandler descriptorSet(compute);
    descriptorSet.Push("preIntegratedDFG", brdfImage.get());
    descriptorSet.Update(compute);

    // Runs the compute pipeline.
    descriptorSet.BindDescriptor(commandBuffer, compute);
    compute.CmdRender(commandBuffer, brdfImage->GetSize());
    commandBuffer.SubmitIdle();

    return brdfImage;
}

std::unique_ptr<ImageCube> StereoRayTracingSubrender::ComputeIrradiance(const std::shared_ptr<ImageCube>& source, uint32_t size)
{
    if (!source) {
        return nullptr;
    }

    auto irradianceCubemap = std::make_unique<ImageCube>(glm::ivec2(size), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);

    // Creates the pipeline.
    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("Shader/Skybox/Irradiance.comp");

    // Bind the pipeline.
    compute.BindPipeline(commandBuffer);

    // Updates descriptors.
    DescriptorsHandler descriptorSet(compute);
    descriptorSet.Push("outColour", irradianceCubemap.get());
    descriptorSet.Push("samplerColour", source);
    descriptorSet.Update(compute);

    // Runs the compute pipeline.
    descriptorSet.BindDescriptor(commandBuffer, compute);
    compute.CmdRender(commandBuffer, irradianceCubemap->GetSize());
    commandBuffer.SubmitIdle();

    return irradianceCubemap;
}

std::unique_ptr<ImageCube> StereoRayTracingSubrender::ComputePrefiltered(const std::shared_ptr<ImageCube>& source, uint32_t size)
{
    if (!source) {
        return nullptr;
    }

    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto prefilteredCubemap = std::make_unique<ImageCube>(glm::ivec2(size),
                                                          VK_FORMAT_R16G16B16A16_SFLOAT,
                                                          VK_IMAGE_LAYOUT_GENERAL,
                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                          VK_FILTER_LINEAR,
                                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                          VK_SAMPLE_COUNT_1_BIT,
                                                          true,
                                                          true);

    // Creates the pipeline.
    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("Shader/Skybox/Prefiltered.comp");

    DescriptorsHandler descriptorSet(compute);
    PushHandler        pushHandler(*compute.GetShader()->GetUniformBlock("PushObject"));

    // TODO: Use image barriers between rendering (single command buffer), rework write descriptor passing. Image class also needs a restructure.
    for (uint32_t i = 0; i < prefilteredCubemap->GetMipLevels(); i++) {
        VkImageView levelView = VK_NULL_HANDLE;
        Image::CreateImageView(prefilteredCubemap->GetImage(),
                               levelView,
                               VK_IMAGE_VIEW_TYPE_CUBE,
                               prefilteredCubemap->GetFormat(),
                               VK_IMAGE_ASPECT_COLOR_BIT,
                               1,
                               i,
                               6,
                               0);

        commandBuffer.Begin();
        compute.BindPipeline(commandBuffer);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler               = prefilteredCubemap->GetSampler();
        imageInfo.imageView             = levelView;
        imageInfo.imageLayout           = prefilteredCubemap->GetLayout();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet               = VK_NULL_HANDLE;   // Will be set in the descriptor handler.
        descriptorWrite.dstBinding           = *compute.GetShader()->GetDescriptorLocation("outColour").second;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorCount      = 1;
        descriptorWrite.descriptorType =
            *compute.GetShader()->GetDescriptorType(*compute.GetShader()->GetDescriptorLocation("outColour").first, descriptorWrite.dstBinding);
        // descriptorWrite.pImageInfo = &imageInfo;
        WriteDescriptorSet writeDescriptorSet(descriptorWrite, imageInfo);

        pushHandler.Push("roughness", static_cast<float>(i) / static_cast<float>(prefilteredCubemap->GetMipLevels() - 1));

        descriptorSet.Push("PushObject", pushHandler);
        descriptorSet.Push("outColour", prefilteredCubemap.get(), std::move(writeDescriptorSet));
        descriptorSet.Push("samplerColour", source);
        descriptorSet.Update(compute);

        descriptorSet.BindDescriptor(commandBuffer, compute);
        pushHandler.BindPush(commandBuffer, compute);
        compute.CmdRender(commandBuffer, prefilteredCubemap->GetSize() >> i);
        commandBuffer.SubmitIdle();

        vkDestroyImageView(*logicalDevice, levelView, nullptr);
    }

    return prefilteredCubemap;
}
}   // namespace MapleLeaf