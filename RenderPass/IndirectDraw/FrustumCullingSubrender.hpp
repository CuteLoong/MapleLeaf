#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "IndirectBuffer.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Resources.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class FrustumCullingSubrender : public Subrender
{
public:
    explicit FrustumCullingSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;
    PipelineCompute  compute;

    DescriptorsHandler descriptorSetCompute;
    DescriptorsHandler descriptorSetGraphics;

    PushHandler    pushHandler;
    UniformHandler uniformCamera;
    UniformHandler uniformCameraCompute;
};
}   // namespace MapleLeaf