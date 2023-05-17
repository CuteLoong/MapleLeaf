#include "MainRenderer.hpp"

#include "Graphics.hpp"
#include "RenderStage.hpp"

namespace Test {
class TestSubrender : public Subrender
{
public:
    explicit TestSubrender(const Pipeline::Stage& stage)
        : Subrender(stage)
    {}

    void Render(const CommandBuffer& commandBuffer) override {}
};

MainRenderer::MainRenderer()
{
    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments{{0, "depth", Attachment::Type::Depth, false},
                                                  {1, "swapchain", Attachment::Type::Swapchain, false},
                                                  {2, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses = {{0, {2}}, {1, {0, 1}}};

    AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments, renderpassSubpasses));
}

void MainRenderer::Start()
{
    AddSubrender<TestSubrender>({0, 0});
}

void MainRenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test