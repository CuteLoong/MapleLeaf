#include "IndirectDrawSubrender.hpp"

namespace MapleLeaf {
IndirectDrawSubrender::IndirectDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"F:/MapleLeaf/Resources/Shader/GPUDriven/Test.vert", "F:/MapleLeaf/Resources/Shader/GPUDriven/Test.frag"}, {}, {},
               PipelineGraphics::Mode::MRT)
{}

void IndirectDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    
}
}   // namespace MapleLeaf