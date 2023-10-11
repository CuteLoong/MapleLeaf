#include "GaussianBlurSubrender.hpp"

#include "Graphics.hpp"
#include "Maths.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
GaussianBlurSubrender::GaussianBlurSubrender(const Pipeline::Stage& pipelineStage, std::string filterTextureName, const GaussianData& gaussianData)
    : Subrender(pipelineStage)
    , filterTextureName(filterTextureName)
    , gaussianData(gaussianData)
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/Blur/GaussianBlur.vert", "Shader/Blur/GaussianBlur.frag"}, {}, {},
                                PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None))
{
    weights.resize(gaussianData.radius * gaussianData.radius);
    float sum = 0.0f;

    for (int x = 0; x < gaussianData.radius; x++) {
        for (int y = 0; y < gaussianData.radius; y++) {
            int index = x * gaussianData.radius + y;
            int u     = x - gaussianData.radius / 2;
            int v     = y - gaussianData.radius / 2;

            float sigmaSquared = gaussianData.sigma * gaussianData.sigma;
            float p            = -(u * u + v * v) / (2 * sigmaSquared);
            float e            = std::exp(p);

            float a = 2 * M_PI * sigmaSquared;

            weights[index] = e / a;
            sum += weights[index];
        }
    }

    for (int i = 0; i < weights.size(); i++) {
        weights[i] /= sum;
    }
}

void GaussianBlurSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void GaussianBlurSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("pixelSize", camera->GetPixelSize());

    storageWeights.Push(weights.data(), sizeof(float) * weights.size());
    uniformGaussian.Push("radius", gaussianData.radius);

    descriptorSet.Push("UniformGaussian", uniformGaussian);
    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("BufferWeight", storageWeights);

    descriptorSet.Push("inTexture", Graphics::Get()->GetAttachment(filterTextureName));

    if (!descriptorSet.Update(pipeline)) return;

    pipeline.BindPipeline(commandBuffer);

    // Draws the object.
    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void GaussianBlurSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf