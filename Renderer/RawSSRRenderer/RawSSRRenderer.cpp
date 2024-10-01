#include "RawSSRRenderer.hpp"

#include "DeferredStereoSubrender.hpp"
#include "GaussianBlurSubrender.hpp"
#include "GaussianBlurXYSubrender.hpp"
#include "Graphics.hpp"
#include "HBAOStereoAwareLeftSubrender.hpp"
#include "HBAOStereoAwareSubrender.hpp"
#include "HBAOStereoSubrender.hpp"
#include "HBAOStereoWithOcclusionInfoSubrender.hpp"
#include "HBAOStereoWithThickSubrender.hpp"
#include "HiZDrawSubrender.hpp"
#include "ImguiSubrender.hpp"
#include "IndirectDrawPrevMV.hpp"
#include "IndirectDrawStereoBackSubrender.hpp"
#include "IndirectDrawStereoSubrender.hpp"
#include "InterpolationBackWarpSubrender.hpp"
#include "InterpolationSubrender.hpp"
#include "MeshesSubrender.hpp"
#include "NonRTAttachmentsHandler.hpp"
#include "PipelineGraphics.hpp"
#include "RawSSRFilterSubrender.hpp"
#include "RawSSRStereoSubrender.hpp"
#include "RenderStage.hpp"
#include "ResolvedSubrender.hpp"
#include "SSRStereoSpatialFilterMultiSPPSubrender.hpp"
#include "SSRStereoSubrender.hpp"
#include "ShadowSubrender.hpp"
#include "SkyboxSubrender.hpp"
#include "StereoMaskSubrender.hpp"
#include "StochasticSSRStereoMultiSPPSubrender.hpp"
#include "ToneMapingSubrender.hpp"

namespace Test {
RawSSRRenderer::RawSSRRenderer()
{
    std::vector<NonRTAttachment> globalAttachments = {
        {"Hi-z", NonRTAttachment::Type::ImageHierarchyZ, false},
        {"MinHi-z", NonRTAttachment::Type::ImageHierarchyZ, false},
        {"MaxHi-z", NonRTAttachment::Type::ImageHierarchyZ, false},
        {"LeftMinHi-z", NonRTAttachment::Type::StereoImageHierarchyZ, false},
        {"RightMinHi-z", NonRTAttachment::Type::StereoImageHierarchyZ, false},
        {"LeftMaxHi-z", NonRTAttachment::Type::StereoImageHierarchyZ, false},
        {"RightMaxHi-z", NonRTAttachment::Type::StereoImageHierarchyZ, false},
        {"MultiSSRColorMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"SSRHitsMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"SSRMask", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R8G8B8A8_UNORM},
        {"GlossyMV", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"ReflectionMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"ReprojectionReflectionColorMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"BRDFWeightMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"TemporalSSRColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"DebugMask", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
        {"PrevSSRColor", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT},
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
        {"Alpha2OneMV", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R16G16B16A16_SFLOAT}};

    CreateGlobalAttachmentsHanlder(globalAttachments);

    // Render Pass for shadow map
    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));

    // Render Pass for previous motion vector
    std::vector<Attachment>  renderpassPrevMVAttachments{{0, "prevDepth", Attachment::Type::Depth, false},
                                                         {1, "prevMotionVector", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT}};
    std::vector<SubpassType> renderpassPrevMVSubpasses = {{0, {}, {0, 1}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassPrevMVAttachments, renderpassPrevMVSubpasses));

    // Render Pass for G-Buffer, skybox and Hi-z min
    std::vector<Attachment> renderpassAttachments1{{0, "depth", Attachment::Type::Depth, false},
                                                   {1, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {3, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {4, "material", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {5, "motionVector", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {6, "instanceId", Attachment::Type::Image, false, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST}};

    std::vector<SubpassType> renderpassSubpasses1 = {{0, {}, {1, 2, 3, 4}}, {1, {}, {0, 1, 2, 3, 4, 5, 6}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassAttachments1, renderpassSubpasses1));

    // Render Pass for Hi-z max
    std::vector<Attachment>  renderpassAttachments2{{0, "backDepth", Attachment::Type::Depth, false}};
    std::vector<SubpassType> renderpassSubpasses2 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassAttachments2, renderpassSubpasses2));

    // Render Pass for derffered lighting
    std::vector<Attachment>  renderpassAttachments3{{0, "lighting", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT}};
    std::vector<SubpassType> renderpassSubpasses3 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments3, renderpassSubpasses3));

    // Render Pass for interpolation
    std::vector<Attachment>  InterpolationAttachment{{0, "PlaceHolder_interpolation", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};
    std::vector<SubpassType> InterpolationSubpasses = {{0, {}, {}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, InterpolationAttachment, InterpolationSubpasses));

    // Render Pass for SSR Filter
    std::vector<Attachment> renderpassAttachments6{{0, "PlaceHolder", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses6 = {{0, {}, {}}, {1, {}, {}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments6, renderpassSubpasses6));

    // Render Pass for SSR resolve and GUI
    std::vector<Attachment> renderpassAttachments7{{0, "resolved", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {1, "indirectLighting", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "swapchain", Attachment::Type::Swapchain, false}};

    std::vector<SubpassType> renderpassSubpasses7 = {{0, {}, {0, 1}}, {1, {0}, {2}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments7, renderpassSubpasses7));
}

void RawSSRRenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});

    AddSubrender<IndirectDrawPrevMV>({1, 0});

    // AddSubrender<SkyboxSubrender>({2, 0});
    AddSubrender<IndirectDrawStereoSubrender>({2, 1});       // Hi-z min and G-Buffer
    AddSubrender<IndirectDrawStereoBackSubrender>({3, 0});   // Hi-z max
    AddSubrender<DeferredStereoSubrender>({4, 0});

    AddSubrender<InterpolationBackWarpSubrender>({5, 0});
    // AddSubrender<InterpolationSubrender>({5, 0});

    AddSubrender<RawSSRStereoSubrender>({6, 0});
    AddSubrender<RawSSRFilterSubrender>({6, 1});

    AddSubrender<ResolvedSubrender>({7, 0});
    AddSubrender<ToneMapingSubrender>({7, 1});
    // AddSubrender<ImguiSubrender>({7, 1});
}

void RawSSRRenderer::Update()
{
    GetNonRTAttachmentsHandler()->Update();
}
}   // namespace Test