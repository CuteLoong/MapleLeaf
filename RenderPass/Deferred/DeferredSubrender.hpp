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
        Color     color;
        glm::vec3 position;
        glm::vec3 attenuation;
    };

    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformScene;
    StorageHandler     storageLights;
};
}   // namespace MapleLeaf