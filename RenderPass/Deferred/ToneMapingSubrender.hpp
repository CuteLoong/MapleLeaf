#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class ToneMapingSubrender : public Subrender
{
public:
    struct ToneMappingInfo
    {
        float exposure;
        float gamma;

        ToneMappingInfo(float exposure = 1.0f, float gamma = 2.2f)
            : exposure(exposure)
            , gamma(gamma)
        {}
    };
    explicit ToneMapingSubrender(const Pipeline::Stage& pipelineStage, ToneMappingInfo toneMappingInfo = ToneMappingInfo());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformToneMaping;

    ToneMappingInfo toneMappingInfo;
};
}   // namespace MapleLeaf