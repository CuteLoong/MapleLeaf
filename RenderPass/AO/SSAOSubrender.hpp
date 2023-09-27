#pragma once

#include "Future.hpp"
#include "Image2d.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"

namespace MapleLeaf {
class SSAOSubrender : public Subrender
{
public:
    explicit SSAOSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;
    Future<std::shared_ptr<Image2d>> noise;
};
}   // namespace MapleLeaf