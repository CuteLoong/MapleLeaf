#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"
#include "glm/glm.hpp"


namespace MapleLeaf {
class DeferredSubrender : public Subrender
{
public:
    explicit DeferredSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    struct PointLight
    {
        Color color = Color::White;
        alignas(16) glm::vec3 position = glm::vec3(0.0f);
        alignas(16) glm::vec3 attenuation = glm::vec3(0.0f);
    };

    struct DirectionalLight
    {
        Color color = Color::White;
        alignas(16) glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    };

    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformScene;
    StorageHandler     storagePointLights;
    StorageHandler     storageDirectionalLights;
};
}   // namespace MapleLeaf