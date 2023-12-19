#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineRayTracing.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class RayTracingSubrender : public Subrender
{
public:
    struct SceneDescription
    {
        uint64_t vertexAddress;         // Address of the Vertex buffer
        uint64_t indexAddress;          // Address of the index buffer
        uint64_t materialAddress;       // Address of the material buffer
        uint64_t instanceInfoAddress;   // Address of the instance info buffer
    };
    explicit RayTracingSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineRayTracing pipelineRayTracing;

    UniformHandler     uniformSceneData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    SceneDescription sceneDescription;
};
}   // namespace MapleLeaf