#include "GPUPipelineResolveSubrender.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
GPUPipelineResolveSubrender::GPUPipelineResolveSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/Deferred/Resolved.vert", "Shader/Deferred/GPUPipelineResolved.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
               PipelineGraphics::Depth::None)
// , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void GPUPipelineResolveSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void GPUPipelineResolveSubrender::Render(const CommandBuffer& commandBuffer)
{
    descriptorSet.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void GPUPipelineResolveSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::unique_ptr<Image2d> GPUPipelineResolveSubrender::ComputeBRDF(uint32_t size)
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