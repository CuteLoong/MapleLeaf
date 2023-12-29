#include "ResolvedSubrender.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
ResolvedSubrender::ResolvedSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/Deferred/Resolved.vert", "Shader/Deferred/Resolved.frag"}, {}, {}, PipelineGraphics::Mode::MRT,
               PipelineGraphics::Depth::None)
    , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void ResolvedSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void ResolvedSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    // Updates storage buffers.
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSet.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSet.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));
    descriptorSet.Push("ReflectionColorMap", Graphics::Get()->GetNonRTAttachment("ReflectionMap"));
    descriptorSet.Push("PreIntegratedBRDF", *brdf);
    descriptorSet.Push("GlossyMV", Graphics::Get()->GetNonRTAttachment("GlossyMV"));

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void ResolvedSubrender::PostRender(const CommandBuffer& commandBuffer) {
    
}

std::unique_ptr<Image2d> ResolvedSubrender::ComputeBRDF(uint32_t size)
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