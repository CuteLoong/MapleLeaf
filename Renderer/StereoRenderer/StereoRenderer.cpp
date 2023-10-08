#include "StereoRenderer.hpp"

#include "DeferredSubrender.hpp"
#include "Graphics.hpp"
#include "HiZDrawSubrender.hpp"
#include "ImguiSubrender.hpp"
#include "IndirectDrawStereoSubrender.hpp"
#include "MeshesSubrender.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "SSAOStereoSubrender.hpp"
#include "ShadowSubrender.hpp"
#include "StereoMaskSubrender.hpp"

namespace Test {
StereoRenderer::StereoRenderer()
{
    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));

    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments1{{0, "depth", Attachment::Type::Depth, false},
                                                   {1, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {3, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                   {4, "material", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses1 = {{0,
                                                      {},
                                                      {
                                                          0,
                                                          1,
                                                          2,
                                                          3,
                                                          4,
                                                      }}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::STEREO, renderpassAttachments1, renderpassSubpasses1));

    std::vector<Attachment> renderpassAttachments2{{0, "swapchain", Attachment::Type::Swapchain, false},
                                                   {1, "resolved", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {2, "AOMap", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                   {3, "StereoMask", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses2 = {{0, {}, {3}}, {1, {}, {2}}, {2, {2}, {0}}, {3, {}, {0}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments2, renderpassSubpasses2));
}

void StereoRenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});
    AddSubrender<IndirectDrawStereoSubrender>({1, 0});

    AddSubrender<StereoMaskSubrender>({2, 0});
    AddSubrender<SSAOStereoSubrender>({2, 1});
    AddSubrender<DeferredSubrender>({2, 2});
    AddSubrender<ImguiSubrender>({2, 3});
}

void StereoRenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test