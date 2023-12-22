#pragma once

#include "Color.hpp"
#include "DescriptorHandler.hpp"
#include "PipelineRayTracing.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class RayTracingSubrender : public Subrender
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
    explicit RayTracingSubrender(const Pipeline::Stage& pipelineStage);

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

    uint32_t frameID = 0;
};
}   // namespace MapleLeaf