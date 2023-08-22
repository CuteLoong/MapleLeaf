#include "ShadowSubrender.hpp"
#include "Scenes.hpp"
#include "ShadowRender.hpp"
#include "Vertex.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
ShadowSubrender::ShadowSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"F:/MapleLeaf/Resources/Shader/ShadowMap/ShadowMap.vert", "F:/MapleLeaf/Resources/Shader/ShadowMap/ShadowMap.frag"}, {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::Polygon)
{}

void ShadowSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    pipeline.BindPipeline(commandBuffer);

    auto sceneShadowRenders = Scenes::Get()->GetScene()->GetComponents<ShadowRender>();

    for (const auto& shadowRender : sceneShadowRenders) shadowRender->CmdRender(commandBuffer, pipeline);
}
}   // namespace MapleLeaf