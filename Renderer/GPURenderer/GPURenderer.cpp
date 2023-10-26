#include "GPURenderer.hpp"

#include "DeferredSubrender.hpp"
#include "GaussianBlurSubrender.hpp"
#include "Graphics.hpp"
#include "HBAOSubrender.hpp"
#include "HiZDrawSubrender.hpp"
#include "ImguiSubrender.hpp"
#include "IndirectDrawSubrender.hpp"
#include "MeshesSubrender.hpp"
#include "NonRTAttachmentsHandler.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "SSAOSubrender.hpp"
#include "ShadowSubrender.hpp"


namespace Test {
GPURenderer::GPURenderer()
{

    std::vector<NonRTAttachment> globalAttachments = {{"Hi-z", NonRTAttachment::Type::ImageHierarchyZ, false}};
    CreateGlobalAttachmentsHanlder(globalAttachments);


    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));

    std::vector<Attachment> renderpassAttachments1{{0, "depthToHiz", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses1 = {{0,
                                                      {},
                                                      {
                                                          0,
                                                      }}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments1, renderpassSubpasses1));


    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments2{{0, "depth", Attachment::Type::Depth, false},
                                                   {1, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {3, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {4, "material", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses2 = {{0,
                                                      {},
                                                      {
                                                          0,
                                                          1,
                                                          2,
                                                          3,
                                                          4,
                                                      }}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments2, renderpassSubpasses2));

    std::vector<Attachment> renderpassAttachments3{{0, "AOMap", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses3 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments3, renderpassSubpasses3));

    std::vector<Attachment> renderpassAttachments4{{0, "AOMapFilter", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses4 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments4, renderpassSubpasses4));

    std::vector<Attachment> renderpassAttachments5{
        {0, "swapchain", Attachment::Type::Swapchain, false},
    };

    std::vector<SubpassType> renderpassSubpasses5 = {{0, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments5, renderpassSubpasses5));
}

void GPURenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});

    // Two_pass
    AddSubrender<HiZDrawSubrender>({1, 0});
    AddSubrender<IndirectDrawSubrender>({2, 0});

    AddSubrender<HBAOSubrender>({3, 0});
    AddSubrender<GaussianBlurSubrender>({4, 0}, "AOMap");
    AddSubrender<DeferredSubrender>({5, 0});
    AddSubrender<ImguiSubrender>({5, 0});
}

void GPURenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
    GetNonRTAttachmentsHandler()->Update();
}
}   // namespace Test