#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class SSRSubrender : public Subrender
{
public:
    struct SSRData
    {
        float maxRayLength;
        float maxSteps;
        float zThickness;
        float fadeValue;

        SSRData(int maxRayLength = 120.0f, float maxSteps = 120.0f, float zThickness = 0.1f, float fadeValue = 0.1f)
            : maxRayLength(maxRayLength)
            , maxSteps(maxSteps)
            , zThickness(zThickness)
            , fadeValue(fadeValue)
        {}
    };

    explicit SSRSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData = SSRData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineCompute;

    UniformHandler     uniformSSRData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    SSRData ssrData;

    static std::vector<Shader::Define> GetDefines();
};
}   // namespace MapleLeaf