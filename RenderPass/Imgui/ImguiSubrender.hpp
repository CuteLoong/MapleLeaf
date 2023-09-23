#pragma once

#include "DescriptorHandler.hpp"
#include "Graphics.hpp"
#include "Imgui.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class ImguiSubrender : public Subrender
{
public:
    explicit ImguiSubrender(const Pipeline::Stage& pipelineStage);

    void Render(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformScaleTranslate;
};
}   // namespace MapleLeaf