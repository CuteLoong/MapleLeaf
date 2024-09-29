#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
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

private:
    PipelineCompute pipelineWarpDepth;
    PipelineCompute pipelineWarpColor;
    PipelineCompute pipelineBlend;

    DescriptorsHandler descriptorSetWarpDepth;
    DescriptorsHandler descriptorSetWarpColor;
    DescriptorsHandler descriptorSetBlend;

    PushHandler    pushHandler;
    UniformHandler uniformCameraWarpDepth;
    UniformHandler uniformCameraWarpColor;
    UniformHandler uniformCameraBlend;
};
}   // namespace MapleLeaf