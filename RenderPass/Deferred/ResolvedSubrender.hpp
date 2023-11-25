#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class ResolvedSubrender : public Subrender
{
public:
    explicit ResolvedSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
};
}   // namespace MapleLeaf