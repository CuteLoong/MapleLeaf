#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "PushHandler.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MONO_Subrender {
using namespace MapleLeaf;

class BackwardFindSubrender : public Subrender
{
public:
    explicit BackwardFindSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

    void ComputeBackward();

private:
    PipelineCompute pipelineBackwardFind;

    DescriptorsHandler descriptorSetBackwardFind;
    UniformHandler     uniformCameraBackwardFind;
    PushHandler        pushHandlerBackwardFind;

    uint32_t frameID = 0;
};
}   // namespace MONO_Subrender