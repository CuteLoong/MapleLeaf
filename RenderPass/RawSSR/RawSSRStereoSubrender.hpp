#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "HaltonSamplePattern.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class RawSSRStereoSubrender : public Subrender
{
public:
    struct SSRData
    {
        float maxRayLength;
        float maxSteps;
        float zThickness;
        float brdfBias;

        SSRData(int maxRayLength = 120.0f, float maxSteps = 240.0f, float zThickness = 0.01f, float brdfBias = 0.7f)
            : maxRayLength(maxRayLength)
            , maxSteps(maxSteps)
            , zThickness(zThickness)
            , brdfBias(brdfBias)
        {}
    };

    explicit RawSSRStereoSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData = SSRData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineCompute;

    UniformHandler     uniformSSRData;
    UniformHandler     uniformJitterData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    SSRData ssrData;

    std::shared_ptr<HaltonSamplePattern> haltonSampler;
    Future<std::shared_ptr<Image2d>>     blueNoise;
    static std::shared_ptr<Image2d>      LoadBlueNoise();
};
}   // namespace MapleLeaf