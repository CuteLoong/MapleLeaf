#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "PushHandler.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class InterpolationSubrender : public Subrender
{
public:
    explicit InterpolationSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

    void ComputeBlend();

private:
    PipelineCompute pipelineWarpDepth;
    PipelineCompute pipelineWarpColor;
    PipelineCompute pipelineBlend;
    PipelineCompute pipelineFinement;

    DescriptorsHandler descriptorSetWarpDepth;
    DescriptorsHandler descriptorSetWarpColor;
    DescriptorsHandler descriptorSetBlend;
    DescriptorsHandler descriptorSetFinement;

    UniformHandler uniformCameraWarpDepth;
    UniformHandler uniformCameraWarpColor;
    UniformHandler uniformCameraBlend;
    UniformHandler uniformCameraFinement;

    PushHandler pushHandlerWarpDepth;
    PushHandler pushHandlerWarpColor;
    PushHandler pushHandlerBlend;

    uint32_t frameID = 0;
};
}   // namespace MapleLeaf