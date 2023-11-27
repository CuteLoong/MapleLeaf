#include "ImguiSubrender.hpp"

namespace MapleLeaf {
ImguiSubrender::ImguiSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/Imgui/ui.vert", "Shader/Imgui/ui.frag"}, {Imgui::GetVertexInput()}, {}, PipelineGraphics::Mode::Imgui,
               PipelineGraphics::Depth::None, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
               VK_FRONT_FACE_COUNTER_CLOCKWISE)
{}

void ImguiSubrender::PreRender(const CommandBuffer& commandBuffer) {}

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

void ImguiSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf