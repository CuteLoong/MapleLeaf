#include "MainRenderer.hpp"

#include "Graphics.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"

namespace Test {
class TestSubrender : public Subrender
{
public:
    explicit TestSubrender(const Pipeline::Stage& stage)
        : Subrender(stage)
        , pipeline(stage, {"E:/MapleLeaf/Resources/Shader/tri1.vert", "E:/MapleLeaf/Resources/Shader/tri1.frag"}, {}, {},
                   PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None)
    {}

    void Render(const CommandBuffer& commandBuffer) override
    {
        pipeline.BindPipeline(commandBuffer);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

private:
    PipelineGraphics pipeline;
};

MainRenderer::MainRenderer()
{
    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments{
        {0, "swapchain", Attachment::Type::Swapchain, false},
    };

    std::vector<SubpassType> renderpassSubpasses = {{0, {0}}};

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