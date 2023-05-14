#include "MainRenderer.hpp"

#include "Graphics.hpp"
#include "RenderStage.hpp"
#include <iostream>

namespace Test {
MainRenderer::MainRenderer()
{
    // attachment 一定不能跨index,必须要是连续的
    std::vector<Attachment> renderpassAttachments{{0, "depth", Attachment::Type::Depth, false},
                                                  {1, "swapchain", Attachment::Type::Swapchain, false},
                                                  {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses = {{0, {2}}, {1, {0, 1}}};

    AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments, renderpassSubpasses));
}

void MainRenderer::Start()
{
    std::cout << "Main Renderer Start" << std::endl;
}

void MainRenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test