#include "ShadowSubrender.hpp"
#include "Scenes.hpp"
#include "Vertex.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
ShadowSubrender::ShadowSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"", ""}, {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None)
{}

void ShadowSubrender::Render(const CommandBuffer& commandBuffer)
{
    pipeline.BindPipeline(commandBuffer);

}
}   // namespace MapleLeaf