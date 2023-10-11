#pragma once

#include "DescriptorHandler.hpp"
#include "Image2d.hpp"
#include "ImageHierarchyZ.hpp"
#include "PipelineCompute.hpp"
#include "Subrender.hpp"

namespace MapleLeaf {
class HiZDrawSubrender : public Subrender
{
public:
    explicit HiZDrawSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipeline;

    DescriptorsHandler descriptorSet;
    PushHandler        pushHandler;

    std::unique_ptr<ImageHierarchyZ> hiz;
    std::unique_ptr<Image2d>         hierarchyDepth;
};
}   // namespace MapleLeaf