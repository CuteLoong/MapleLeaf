#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class HBAOSubrender : public Subrender
{
public:
    struct HBAOData
    {
        glm::uvec2 noiseScale;
        uint32_t   numRays;
        float      strengthPerRay;
        uint32_t   maxStepsPerRay;
        float      sampleRadius;

        HBAOData(glm::uvec2 noiseScale = glm::uvec2{1.0f, 1.0f}, uint32_t numRays = 8, float strengthPerRay = 0.1875f, uint32_t maxStepsPerRay = 5,
                 float sampleRadius = 1.0f)
            : noiseScale(noiseScale)
            , numRays(numRays)
            , strengthPerRay(strengthPerRay)
            , maxStepsPerRay(maxStepsPerRay)
            , sampleRadius(sampleRadius)
        {}
    };

    explicit HBAOSubrender(const Pipeline::Stage& pipelineStage, HBAOData hbaoData = HBAOData());

    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    UniformHandler     uniformControl;
    UniformHandler     uniformScene;
    DescriptorsHandler descriptorSet;

    HBAOData hbaoData;

    glm::uvec2                       noiseSize;
    Future<std::shared_ptr<Image2d>> hbaoNoise;   // cos(Angle), sin(Angle), firstStepNoise.x, firstStepNoise.y

    static std::shared_ptr<Image2d> ComputeNoise(uint32_t width, uint32_t height, uint32_t numRays);
};
}   // namespace MapleLeaf