#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class IndirectDrawSubrender : public Subrender
{
public:
    explicit IndirectDrawSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    // DescriptorsHandler descriptorSet;
};
}   // namespace MapleLeaf