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
class IndirectDrawSubrender : public Subrender
{
public:
    explicit IndirectDrawSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics   pipeline;
    DescriptorsHandler descriptorSet;

    UniformHandler uniformScene;
};
}   // namespace MapleLeaf