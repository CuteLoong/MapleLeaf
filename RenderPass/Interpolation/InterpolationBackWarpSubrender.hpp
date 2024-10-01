#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "PushHandler.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class InterpolationBackWarpSubrender : public Subrender
{
public:
    explicit InterpolationBackWarpSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

    void ComputeBlend();

private:
    PipelineCompute pipelineWarpDepth;
    PipelineCompute pipelineWarpMV;
    PipelineCompute pipelineBlend;

    DescriptorsHandler descriptorSetWarpDepth;
    DescriptorsHandler descriptorSetWarpMV;
    DescriptorsHandler descriptorSetBlend;

    UniformHandler uniformCameraWarpDepth;
    UniformHandler uniformCameraWarpMV;
    UniformHandler uniformCameraBlend;

    PushHandler pushHandlerWarpDepth;
    PushHandler pushHandlerWarpMV;
    PushHandler pushHandlerBlend;

    uint32_t frameID = 0;
};
}   // namespace MapleLeaf