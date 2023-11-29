#include "SkyboxSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
SkyboxSubrender::SkyboxSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineGraphics(pipelineStage, {"Shader/Skybox/Skybox.vert", "Shader/Skybox/Skybox.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
                       PipelineGraphics::Depth::None)
    , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void SkyboxSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void SkyboxSubrender::Render(const CommandBuffer& commandBuffer) {}

void SkyboxSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::unique_ptr<Image2d> SkyboxSubrender::ComputeBRDF(uint32_t size)
{
    auto brdfImage = std::make_unique<Image2d>(glm::uvec2(size), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);

    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("Shader/Skybox/PreIntegrationDFG.comp");

    // Bind the pipeline.
    compute.BindPipeline(commandBuffer);

    // Updates descriptors.
    DescriptorsHandler descriptorSet(compute);
    descriptorSet.Push("preIntegratedDFG", brdfImage.get());
    descriptorSet.Update(compute);

    // Runs the compute pipeline.
    descriptorSet.BindDescriptor(commandBuffer, compute);
    compute.CmdRender(commandBuffer, brdfImage->GetSize());
    commandBuffer.SubmitIdle();

    return brdfImage;
}
}   // namespace MapleLeaf