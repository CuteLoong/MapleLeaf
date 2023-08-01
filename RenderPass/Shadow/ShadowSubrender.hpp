#pragma once

#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "DescriptorHandler.hpp"
#include "PushHandler.hpp"

namespace MapleLeaf {
class ShadowSubrender : public Subrender
{
public:
    explicit ShadowSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    PushHandler pushObject;
};
}   // namespace MapleLeaf