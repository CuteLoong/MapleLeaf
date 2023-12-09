#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"
#include "glm/glm.hpp"


namespace MapleLeaf {
class DeferredStereoSubrender : public Subrender
{
public:
    explicit DeferredStereoSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
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

    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformScene;
    UniformHandler     uniformCamera;
    StorageHandler     storagePointLights;
    StorageHandler     storageDirectionalLights;
};
}   // namespace MapleLeaf