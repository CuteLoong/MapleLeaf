#include "HiZDrawSubrender.hpp"

#include "Graphics.hpp"
#include "Image.hpp"


namespace MapleLeaf {
HiZDrawSubrender::HiZDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline("F:/MapleLeaf/Resources/Shader/GPUDriven/HierarchicalDepth.comp")
{}

void HiZDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    glm::ivec2 previousLevelDimensions, currentDimensions;
    int        mipLevel     = 0;
    previousLevelDimensions = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};
    currentDimensions       = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};

    const ImageDepth* depth = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    Image::InsertImageMemoryBarrier(commandBuffer,
                                    depth->GetImage(),
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_MEMORY_READ_BIT,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                    1,
                                    0,
                                    1,
                                    0);

    pipeline.BindPipeline(commandBuffer);

    pushHandler.Push("previousLevelDimensions", previousLevelDimensions);
    pushHandler.Push("currentDimensions", currentDimensions);
    pushHandler.Push("mipLevel", mipLevel);

    descriptorSet.Push("HiZ", Graphics::Get()->GetAttachment("HiZ"));
    descriptorSet.Push("depthBuffer", depth);
    descriptorSet.Push("PushObject", pushHandler);
    if(!descriptorSet.Update(pipeline)) return;

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    pushHandler.BindPush(commandBuffer, pipeline);

    pipeline.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x, currentDimensions.y));

    Image::InsertImageMemoryBarrier(commandBuffer,
                                    depth->GetImage(),
                                    VK_ACCESS_MEMORY_READ_BIT,
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                    1,
                                    0,
                                    1,
                                    0);
}
}   // namespace MapleLeaf