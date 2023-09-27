#include "SSAOSubrender.hpp"
#include "Log.hpp"

namespace MapleLeaf {
SSAOSubrender::SSAOSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(PipelineGraphics(pipelineStage, {"F:/MapleLeaf/Resources/Shader/AO/SSAO.vert", "F:/MapleLeaf/Resources/Shader/AO/SSAO.frag"}, {}, {},
                                PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None))
{}

void SSAOSubrender::Render(const CommandBuffer& commandBuffer) {
    
}


}   // namespace MapleLeaf