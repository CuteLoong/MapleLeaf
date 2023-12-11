#include "SkyboxSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"
#include "Skybox.hpp"

namespace MapleLeaf {
static const Color SKYBOX_COLOUR_DAY(0x003C8A);

SkyboxSubrender::SkyboxSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineGraphics(pipelineStage, {"Shader/Skybox/SkyboxStereo.vert", "Shader/Skybox/SkyboxStereo.geom", "Shader/Skybox/SkyboxStereo.frag"}, {},
                       {}, PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::None, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
                       VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false)
// , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void SkyboxSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void SkyboxSubrender::Render(const CommandBuffer& commandBuffer)
{
    const auto& skybox = Scenes::Get()->GetScene()->GetComponent<Skybox>();
    if (!skybox) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformSkybox.Push("transform", skybox->GetTransform());
    uniformSkybox.Push("baseColour", SKYBOX_COLOUR_DAY);
    uniformSkybox.Push("blendFactor", 1.0f);

    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("UniformSkybox", uniformSkybox);
    descriptorSet.Push("SkyboxCubeMap", skybox->GetImageCube());

    if (!descriptorSet.Update(pipelineGraphics)) return;

    pipelineGraphics.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineGraphics);

    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

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