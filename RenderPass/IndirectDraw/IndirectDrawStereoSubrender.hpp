#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "IndirectBuffer.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Resources.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class IndirectDrawStereoSubrender : public Subrender
{
public:
    explicit IndirectDrawStereoSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;
    PipelineCompute  compute;

    DescriptorsHandler descriptorSetCompute;
    DescriptorsHandler descriptorSetGraphics;

    PushHandler    pushHandler;
    UniformHandler uniformCamera;
    UniformHandler uniformCameraCompute;

    // HiZ Draw Data
    PipelineCompute pipelineComputeHiZMin;

    std::vector<PushHandler> pushHandlers;

    std::vector<DescriptorsHandler> descriptorSetComputeHiZMin;

    std::vector<std::unique_ptr<Image2d>> minHiDepthsLeft;
    std::vector<std::unique_ptr<Image2d>> minHiDepthsRight;

    void RecreateHiDepths();
};
}   // namespace MapleLeaf