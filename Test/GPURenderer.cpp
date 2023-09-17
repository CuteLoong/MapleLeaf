#include "GPURenderer.hpp"

#include "DeferredSubrender.hpp"
#include "Graphics.hpp"
#include "IndirectDrawSubrender.hpp"
#include "MeshesSubrender.hpp"
#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "ShadowSubrender.hpp"



namespace Test {
GPURenderer::GPURenderer()
{
    // // attachment 一定不能跨index,必须要是连续的,不然会导致framebuffer读取view时索引越界,且renderpass的attachment索引也会对不上
    std::vector<Attachment> renderpassAttachments{
        {0, "depth", Attachment::Type::Depth, false},
        {1, "swapchain", Attachment::Type::Swapchain, false},
    };

    std::vector<SubpassType> renderpassSubpasses = {{0, {}, {0, 1}}};

    AddRenderStage(std::make_unique<RenderStage>(renderpassAttachments, renderpassSubpasses));
}

void GPURenderer::Start()
{
    AddSubrender<IndirectDrawSubrender>({0, 0});
}

void GPURenderer::Update()
{
    // std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test