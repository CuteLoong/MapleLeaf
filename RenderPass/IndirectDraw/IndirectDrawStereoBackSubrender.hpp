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
class IndirectDrawStereoBackSubrender : public Subrender
{
public:
    explicit IndirectDrawStereoBackSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSetGraphics;
    PushHandler        pushHandler;
    UniformHandler     uniformCamera;

    // HiZ Draw Data
    PipelineCompute          pipelineComputeHiZMax;
    std::vector<PushHandler> pushHandlers;

    std::vector<DescriptorsHandler> descriptorSetComputeHiZMax;

    std::vector<std::unique_ptr<Image2d>> maxHiDepthsLeft;
    std::vector<std::unique_ptr<Image2d>> maxHiDepthsRight;

    void RecreateHiDepths();
};
}   // namespace MapleLeaf