#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"


namespace MapleLeaf {
class StereoMaskSubrender : public Subrender
{
public:
    explicit StereoMaskSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    UniformHandler     uniformScene;
    DescriptorsHandler descriptorSet;
};
}   // namespace MapleLeaf