#pragma once

#include "Color.hpp"
#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "Image2d.hpp"
#include "ImageCube.hpp"
#include "PipelineRayTracing.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class StereoRayTracingSubrender : public Subrender
{
public:
    struct PointLight
    {
        Color color                       = Color::White;
        alignas(16) glm::vec3 position    = glm::vec3(0.0f);
        alignas(16) glm::vec3 attenuation = glm::vec3(0.0f);
    };

    struct DirectionalLight
    {
        Color color                     = Color::White;
        alignas(16) glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    };
    explicit StereoRayTracingSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineRayTracing pipelineRayTracing;

    UniformHandler     uniformSceneData;
    UniformHandler     uniformFrameData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    // light buffer need light class manager
    StorageHandler storagePointLights;
    StorageHandler storageDirectionalLights;

    std::shared_ptr<ImageCube> skybox;

    Future<std::unique_ptr<Image2d>>   brdf;
    Future<std::unique_ptr<ImageCube>> irradiance;
    Future<std::unique_ptr<ImageCube>> prefiltered;

    static std::unique_ptr<Image2d>   ComputeBRDF(uint32_t size);
    static std::unique_ptr<ImageCube> ComputeIrradiance(const std::shared_ptr<ImageCube>& source, uint32_t size);
    static std::unique_ptr<ImageCube> ComputePrefiltered(const std::shared_ptr<ImageCube>& source, uint32_t size);
};
}   // namespace MapleLeaf