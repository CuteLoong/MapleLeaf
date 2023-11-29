#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class SSRStereoSubrender : public Subrender
{
public:
    struct SSRData
    {
        float maxRayLength;
        float maxSteps;
        float zThickness;
        float fadeValue;

        SSRData(int maxRayLength = 120.0f, float maxSteps = 120.0f, float zThickness = 0.01f, float fadeValue = 0.1f)
            : maxRayLength(maxRayLength)
            , maxSteps(maxSteps)
            , zThickness(zThickness)
            , fadeValue(fadeValue)
        {}
    };

    explicit SSRStereoSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData = SSRData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineCompute;

    UniformHandler     uniformSSRData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    SSRData ssrData;

    Future<std::unique_ptr<Image2d>>   brdf;
    static std::unique_ptr<Image2d>    ComputeBRDF(uint32_t size);
    static std::vector<Shader::Define> GetDefines();
};
}   // namespace MapleLeaf