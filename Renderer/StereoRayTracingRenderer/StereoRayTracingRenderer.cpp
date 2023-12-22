#include "StereoRayTracingRenderer.hpp"

#include "ImguiSubrender.hpp"
#include "RayTracingToneMappingSubrender.hpp"
#include "StereoRayTracingSubrender.hpp"

namespace Test {
StereoRayTracingRenderer::StereoRayTracingRenderer()
{
    std::vector<NonRTAttachment> globalAttachments = {{"RayTracingTarget", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32G32B32A32_SFLOAT}};
    CreateGlobalAttachmentsHanlder(globalAttachments);

    std::vector<Attachment> renderpassAttachments0 = {};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0));

    std::vector<Attachment> renderpassAttachments1 = {{0, "swapchain", Attachment::Type::Swapchain, false}};

    std::vector<SubpassType> renderpassSubpasses1 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments1, renderpassSubpasses1));
}

void StereoRayTracingRenderer::Start()
{
    AddSubrender<StereoRayTracingSubrender>({0, 0});

    AddSubrender<RayTracingToneMappingSubrender>({1, 0});
    AddSubrender<ImguiSubrender>({1, 0});
}

void StereoRayTracingRenderer::Update()
{
    GetNonRTAttachmentsHandler()->Update();
}
}   // namespace Test