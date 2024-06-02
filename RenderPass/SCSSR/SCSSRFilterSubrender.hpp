#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "HaltonSamplePattern.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class SCSSRFilterSubrender : public Subrender
{
public:
    explicit SCSSRFilterSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineCompute;
    PipelineCompute pipelineTemporalFilter;

    UniformHandler uniformFilterData;
    UniformHandler uniformJitterData;
    UniformHandler uniformCamera;
    UniformHandler uniformCameraTemporalFilter;

    DescriptorsHandler descriptorSet;
    DescriptorsHandler descriptorSetTemporalFilter;

    std::shared_ptr<HaltonSamplePattern> haltonSampler;
    Future<std::shared_ptr<Image2d>>     blueNoise;
    static std::shared_ptr<Image2d>      LoadBlueNoise();
};
}   // namespace MapleLeaf