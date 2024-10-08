#include "MainRenderer.hpp"

#include "DeferredSubrender.hpp"
#include "Graphics.hpp"
#include "MeshesSubrender.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "ShadowSubrender.hpp"

namespace Test {
MainRenderer::MainRenderer()
{
    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Depth, false}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {}, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));

    // // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments{{0, "depth", Attachment::Type::Depth, false},
                                                  {1, "swapchain", Attachment::Type::Swapchain, false},
                                                  {2, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                  {3, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                  {4, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
                                                  {5, "material", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
                                                  {6, "resolved", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses = {{0, {}, {0, 2, 3, 4, 5}}, {1, {2, 3, 4, 5}, {0, 1}}};

    AddRenderStage(std::make_unique<RenderStage>(RenderStage::Type::MONO, renderpassAttachments, renderpassSubpasses));
}

void MainRenderer::Start()
{
    AddSubrender<ShadowSubrender>({0, 0});

    AddSubrender<MeshesSubrender>({1, 0});

    AddSubrender<DeferredSubrender>({1, 1});
}

void MainRenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test