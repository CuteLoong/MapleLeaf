#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"


namespace MapleLeaf {
class StereoMaskSubrender : public Subrender
{
public:
    explicit StereoMaskSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    PipelineCompute    pipelineGradient;
    UniformHandler     uniformCameraCompute;
    DescriptorsHandler descriptorSetCompute;
};
}   // namespace MapleLeaf