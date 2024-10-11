#include "WarpRenderer.hpp"

#include "DeferredStereoSubrender.hpp"
#include "DeferredSubrender.hpp"
#include "FrustumCullingSubrender.hpp"
#include "IndirectDrawPrevMV.hpp"
#include "InterpolationMono/InterpolationSubrender.hpp"
#include "ResolvedSubrender.hpp"
#include "ShadowSubrender.hpp"
#include "ToneMapingSubrender.hpp"

namespace Test {

WarpRenderer::WarpRenderer()
{
    std::vector<NonRTAttachment> globalAttachments = {
        {"PrevLighting", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"Zero2OneColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"One2ZeroColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"AlphaColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"Zero2OneDepth_Int", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32_SINT, VK_FILTER_NEAREST},
        {"One2ZeroDepth_Int", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32_SINT, VK_FILTER_NEAREST},
        {"Zero2OneDepth", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32_SFLOAT},
        {"One2ZeroDepth", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32_SFLOAT},
        {"AlphaDepth", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R32_SFLOAT},
        {"Zero2AlphaMV", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"Alpha2OneMV", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"FinedColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"BlockInit", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"BlockZero2Alpha", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"BlockOne2Alpha", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT}};

    CreateGlobalAttachmentsHanlder(globalAttachments);

    // Render Pass 0 for shadow map
    std::vector<Attachment> ShadowAttachments = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> ShadowSubpasses = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, ShadowAttachments, ShadowSubpasses, Viewport({4096, 4096})));

    // Render Pass 1 for previous motion vector
    std::vector<Attachment>  PrevMVAttachments{{0, "prevDepth", Attachment::Type::Depth, false},
                                               {1, "prevMotionVector", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT}};
    std::vector<SubpassType> PrevMVSubpasses = {{0, {}, {0, 1}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, PrevMVAttachments, PrevMVSubpasses));

    // Render Pass 2 for G-Buffer, skybox and Hi-z min
    std::vector<Attachment>  renderpassAttachments1{{0, "depth", Attachment::Type::Depth, false},
                                                    {1, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                    {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                    {3, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                    {4, "material", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                    {5, "motionVector", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                    {6, "instanceId", Attachment::Type::Image, false, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST}};
    std::vector<SubpassType> renderpassSubpasses1 = {{0, {}, {1, 2, 3, 4}}, {1, {}, {0, 1, 2, 3, 4, 5, 6}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments1, renderpassSubpasses1));

    // Render Pass 3 for derffered lighting
    std::vector<Attachment>  renderpassAttachments3{{0, "lighting", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT}};
    std::vector<SubpassType> renderpassSubpasses3 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments3, renderpassSubpasses3));

    // Render Pass 4 for interpolation
    std::vector<Attachment>  InterpolationAttachment{{0, "PlaceHolder_interpolation", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};
    std::vector<SubpassType> InterpolationSubpasses = {{0, {}, {}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, InterpolationAttachment, InterpolationSubpasses));

    // Render Pass 5 for Present
    std::vector<Attachment> renderpassAttachments6{{0, "resolved", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {1, "swapchain", Attachment::Type::Swapchain, false}};

    std::vector<SubpassType> renderpassSubpasses6 = {{0, {}, {0}}, {1, {0}, {1}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments6, renderpassSubpasses6));
}

void WarpRenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});

    AddSubrender<IndirectDrawPrevMV>({1, 0});

    AddSubrender<FrustumCullingSubrender>({2, 1});
    AddSubrender<DeferredSubrender>({3, 0});

    AddSubrender<MONO_Subrender::InterpolationSubrender>({4, 0});

    AddSubrender<WarpRenderer_SubRender::ResolvedSubrender>({5, 0});
    AddSubrender<ToneMapingSubrender>({5, 1});
}

void WarpRenderer::Update()
{
    GetNonRTAttachmentsHandler()->Update();
}
}   // namespace Test