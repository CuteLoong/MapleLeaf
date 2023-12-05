#include "SSRStereoSpatialFilterSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
SSRStereoSpatialFilterSubrender::SSRStereoSpatialFilterSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineCompute("Shader/SSR/SSRStereoSpatialFilter.comp", {}, false)
    , blueNoise(Resources::Get()->GetThreadPool().Enqueue(LoadBlueNoise))
{
    haltonSampler = HaltonSamplePattern::Create(4);
}

void SSRStereoSpatialFilterSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void SSRStereoSpatialFilterSubrender::Render(const CommandBuffer& commandBuffer) {}

void SSRStereoSpatialFilterSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::shared_ptr<Image2d> SSRStereoSpatialFilterSubrender::LoadBlueNoise()
{
    auto blueNoiseImage = Image2d::Create("NoiseImage/BlueNoise.tga", VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, false);

    return blueNoiseImage;
}
}   // namespace MapleLeaf