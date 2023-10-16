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

void GaussianBlurXYSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& gaussianX = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("gaussianX"));
    const auto& gaussianY = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("gaussianY"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene[0].Push("pixelSize", camera->GetPixelSize());

    uniformGaussian[0].Push("radius", gaussianData.radius);

    storageWeights[0].Push(weights.data(), sizeof(float) * weights.size());

    descriptorSet[0].Push("UniformGaussian", uniformGaussian[0]);
    descriptorSet[0].Push("UniformScene", uniformScene[0]);
    descriptorSet[0].Push("BufferWeight", storageWeights[0]);
    descriptorSet[0].Push("inTexture", Graphics::Get()->GetAttachment(filterTextureName));
    descriptorSet[0].Push("GaussianX", gaussianX);

    if (!descriptorSet[0].Update(pipelineX)) return;

    pipelineX.BindPipeline(commandBuffer);

    descriptorSet[0].BindDescriptor(commandBuffer, pipelineX);

    pipelineX.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    gaussianX->Image2dPipelineBarrierComputeToCompute(commandBuffer);

    uniformScene[1].Push("pixelSize", camera->GetPixelSize());

    uniformGaussian[1].Push("radius", gaussianData.radius);

    storageWeights[1].Push(weights.data(), sizeof(float) * weights.size());

    descriptorSet[1].Push("UniformGaussian", uniformGaussian[1]);
    descriptorSet[1].Push("UniformScene", uniformScene[1]);
    descriptorSet[1].Push("BufferWeight", storageWeights[1]);
    descriptorSet[1].Push("inTexture", gaussianX);
    descriptorSet[1].Push("GaussianY", gaussianY);

    if (!descriptorSet[1].Update(pipelineY)) return;

    pipelineY.BindPipeline(commandBuffer);

    descriptorSet[1].BindDescriptor(commandBuffer, pipelineY);

    pipelineY.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    gaussianY->Image2dPipelineBarrierComputeToGraphic(commandBuffer);
}
}   // namespace MapleLeaf