#include "GaussianBlurXYSubrender.hpp"

#include "Graphics.hpp"
#include "Maths.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
GaussianBlurXYSubrender::GaussianBlurXYSubrender(const Pipeline::Stage& pipelineStage, std::string filterTextureName,
                                                 const GaussianData& gaussianData)
    : Subrender(pipelineStage)
    , filterTextureName(filterTextureName)
    , gaussianData(gaussianData)
    , pipelineX("Shader/Blur/GaussianBlurX.comp")
    , pipelineY("Shader/Blur/GaussianBlurY.comp")
{
    weights.resize(gaussianData.radius);
    float sum = 0.0f;

    for (int x = 0; x < gaussianData.radius; x++) {
        int u = x - gaussianData.radius / 2;

        float sigmaSquared = gaussianData.sigma * gaussianData.sigma;
        float p            = -(u * u) / (2 * sigmaSquared);
        float e            = std::exp(p);

        float a = 2 * M_PI * sigmaSquared;

        weights[x] = e / a;
        sum += weights[x];
    }

    for (int i = 0; i < weights.size(); i++) {
        weights[i] /= sum;
    }
}

void GaussianBlurXYSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void GaussianBlurXYSubrender::Render(const CommandBuffer& commandBuffer) {}

void GaussianBlurXYSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf