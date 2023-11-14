#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class HBAOStereoWithThickSubrender : public Subrender
{
public:
    struct HBAOData
    {
        glm::uvec2 noiseScale;
        uint32_t   numRays;
        uint32_t   stepCount;
        float      maxRadiusPixels;
        float      sampleRadius;
        float      intensity;
        float      angleBias;

        HBAOData(glm::uvec2 noiseScale = glm::uvec2{1.0f, 1.0f}, uint32_t numRays = 8, uint32_t stepCount = 5, float maxRadiusPixels = 32.0f,
                 float sampleRadius = 1.5f, float intensity = 2.0f, float angleBias = 0.1f)
            : noiseScale(noiseScale)
            , numRays(numRays)
            , stepCount(stepCount)
            , maxRadiusPixels(maxRadiusPixels)
            , sampleRadius(sampleRadius)
            , intensity(intensity)
            , angleBias(angleBias)
        {}
    };

    explicit HBAOStereoWithThickSubrender(const Pipeline::Stage& pipelineStage, HBAOData hbaoData = HBAOData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    UniformHandler     uniformHBAOData;
    UniformHandler     uniformCamera;
    DescriptorsHandler descriptorSet;

    HBAOData hbaoData;

    glm::uvec2                       noiseSize;
    Future<std::shared_ptr<Image2d>> hbaoNoise;   // cos(Angle), sin(Angle), firstStepNoise.x, firstStepNoise.y

    static std::shared_ptr<Image2d> ComputeNoise(uint32_t width, uint32_t height, uint32_t numRays);
};
}   // namespace MapleLeaf