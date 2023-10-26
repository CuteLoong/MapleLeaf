#pragma once

#include "DescriptorHandler.hpp"
#include "Image2d.hpp"
#include "ImageHierarchyZ.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "PushHandler.hpp"
#include "Subrender.hpp"
#include <memory>
#include <vector>

namespace MapleLeaf {
class HiZDrawSubrender : public Subrender
{
public:
    explicit HiZDrawSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute  pipelineCompute1;
    PipelineCompute  pipelineCompute2;
    PipelineGraphics PipelineGraphics;

    DescriptorsHandler              descriptorSetGraphics;
    DescriptorsHandler              descriptorSetCompute1;
    std::vector<DescriptorsHandler> descriptorSetsCompute;

    PushHandler              pushHandler1;
    PushHandler              pushHandler2;
    PushHandler              pushHandler3;
    std::vector<PushHandler> pushHandlers;

    UniformHandler uniformCamera;
    UniformHandler uniformCameraCompute;

    // std::unique_ptr<ImageHierarchyZ> hiz;
    std::unique_ptr<Image2d> hierarchyDepth;

    std::vector<std::unique_ptr<Image2d>> HiDepths;
};
}   // namespace MapleLeaf