#include "ImguiSubrender.hpp"

namespace MapleLeaf {
ImguiSubrender::ImguiSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"F:/MapleLeaf/Resources/Shader/Imgui/ui.vert", "F:/MapleLeaf/Resources/Shader/Imgui/ui.frag"},
               {Imgui::GetVertexInput()}, {}, PipelineGraphics::Mode::Imgui, PipelineGraphics::Depth::None)
{}

void ImguiSubrender::Render(const CommandBuffer& commandBuffer)
{

    uniformScaleTranslate.Push("scale", Imgui::Get()->GetScale());
    uniformScaleTranslate.Push("translate", Imgui::Get()->GetTranslate());

    descriptorSet.Push("ScaleTransform", uniformScaleTranslate);
    descriptorSet.Push("fontSampler", Imgui::Get()->GetFontImage());

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    Imgui::Get()->cmdRender(commandBuffer);
}
}   // namespace MapleLeaf