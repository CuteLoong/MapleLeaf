#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "Subrender.hpp"

namespace MapleLeaf {
class HiZDrawSubrender : public Subrender
{
public:
    explicit HiZDrawSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipeline;

    DescriptorsHandler descriptorSet;
    PushHandler        pushHandler;
};
}   // namespace MapleLeaf