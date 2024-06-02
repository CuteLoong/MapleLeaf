#include "SCSSRRenderer.hpp"

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
#include "IndirectDrawStereoBackSubrender.hpp"
#include "IndirectDrawStereoSubrender.hpp"
#include "MeshesSubrender.hpp"
#include "NonRTAttachmentsHandler.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "ResolvedSubrender.hpp"
#include "SCSSRFilterSubrender.hpp"
#include "SCSSRReprojectionSubrender.hpp"
#include "SCSSRStereoSubrender.hpp"
#include "SSRStereoSpatialFilterMultiSPPSubrender.hpp"
#include "SSRStereoSubrender.hpp"
#include "ShadowSubrender.hpp"
#include "SkyboxSubrender.hpp"
#include "StereoMaskSubrender.hpp"
#include "StochasticSSRStereoMultiSPPSubrender.hpp"
#include "ToneMapingSubrender.hpp"
#include "vulkan/vulkan_core.h"





namespace Test {
SCSSRRenderer::SCSSRRenderer()
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
        {"ReuseMap", NonRTAttachment::Type::Image2d, false, VK_FORMAT_R8G8B8A8_UNORM}};

    CreateGlobalAttachmentsHanlder(globalAttachments);

    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));

    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments1{{0, "depth", Attachment::Type::Depth, false},
                                                   {1, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {3, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {4, "material", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {5, "instanceId", Attachment::Type::Image, false, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST}};

    std::vector<SubpassType> renderpassSubpasses1 = {{0, {}, {1, 2, 3, 4}}, {1, {}, {0, 1, 2, 3, 4, 5}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassAttachments1, renderpassSubpasses1));

    std::vector<Attachment>  renderpassAttachments2{{0, "backDepth", Attachment::Type::Depth, false}};
    std::vector<SubpassType> renderpassSubpasses2 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassAttachments2, renderpassSubpasses2));

    std::vector<Attachment> renderpassAttachments3{{0, "lighting", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT}};

    std::vector<SubpassType> renderpassSubpasses3 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments3, renderpassSubpasses3));

    std::vector<Attachment> renderpassAttachments4{{0, "PlaceHolder", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses4 = {{0, {}, {}}, {1, {}, {}}, {2, {}, {}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments4, renderpassSubpasses4));

    std::vector<Attachment> renderpassAttachments5{{0, "resolved", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {1, "indirectLighting", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "swapchain", Attachment::Type::Swapchain, false}};

    std::vector<SubpassType> renderpassSubpasses5 = {{0, {}, {0, 1}}, {1, {0}, {2}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments5, renderpassSubpasses5));
}

void SCSSRRenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});

    // AddSubrender<SkyboxSubrender>({1, 0});
    AddSubrender<IndirectDrawStereoSubrender>({1, 1});       // Hi-z min and G-Buffer
    AddSubrender<IndirectDrawStereoBackSubrender>({2, 0});   // Hi-z max
    AddSubrender<DeferredStereoSubrender>({3, 0});

    AddSubrender<SCSSRStereoSubrender>({4, 0});
    AddSubrender<SCSSRReprojectionSubrender>({4, 1});
    AddSubrender<SCSSRFilterSubrender>({4, 2});

    AddSubrender<ResolvedSubrender>({5, 0});
    AddSubrender<ToneMapingSubrender>({5, 1});
    // AddSubrender<ImguiSubrender>({5, 1});
}

void SCSSRRenderer::Update()
{
    GetNonRTAttachmentsHandler()->Update();
}
}   // namespace Test