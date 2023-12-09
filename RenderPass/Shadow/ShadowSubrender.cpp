#include "ShadowSubrender.hpp"
#include "Scenes.hpp"
#include "ShadowRender.hpp"
#include "Vertex.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
ShadowSubrender::ShadowSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/ShadowMap/ShadowMap.vert", "Shader/ShadowMap/ShadowMap.frag"}, {Vertex3D::GetVertexInput()}, {},
               PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::ReadWrite, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
               VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false)
{}

void ShadowSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void ShadowSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    pipeline.BindPipeline(commandBuffer);

    auto sceneShadowRenders = Scenes::Get()->GetScene()->GetComponents<ShadowRender>();

    for (const auto& shadowRender : sceneShadowRenders) shadowRender->CmdRender(commandBuffer, pipeline);
}

void ShadowSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf