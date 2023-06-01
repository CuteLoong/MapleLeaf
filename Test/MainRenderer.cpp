#include "MainRenderer.hpp"

#include "Graphics.hpp"
#include "MeshesSubrender.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"

namespace Test {
class TestSubrender : public Subrender
{
public:
    explicit TestSubrender(const Pipeline::Stage& stage)
        : Subrender(stage)
        , pipeline(stage, {"F:/MapleLeaf/Resources/Shader/tri1.vert", "F:/MapleLeaf/Resources/Shader/tri1.frag"}, {}, {},
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
    std::vector<Attachment> renderpassAttachments0 = {{0, "shadows", Attachment::Type::Image, false, VK_FORMAT_R8_UNORM}};

    std::vector<SubpassType> renderpassSubpasses0 = {{0, {0}}};
    AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments0, renderpassSubpasses0, Viewport({4096, 4096})));
    
    // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments{
        {0, "depth", Attachment::Type::Depth, false},
        {1, "swapchain", Attachment::Type::Swapchain, false},
        {2, "position", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
		{3, "diffuse", Attachment::Type::Image, false, VK_FORMAT_R8G8B8A8_UNORM},
		{4, "normal", Attachment::Type::Image, false, VK_FORMAT_R16G16B16A16_SFLOAT},
    };

    std::vector<SubpassType> renderpassSubpasses = {{0, {0, 2, 3, 4}}, {1, {1}}};

    AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments, renderpassSubpasses));
}

void MainRenderer::Start()
{
    AddSubrender<MeshesSubrender>({1, 0});
    AddSubrender<TestSubrender>({1, 1});
}

void MainRenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test