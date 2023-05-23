#pragma once

#include "PipelineGraphics.hpp"
#include "RenderStage.hpp"
#include "Resource.hpp"

namespace MapleLeaf {
class MaterialPipeline : public Resource
{
public:
    MaterialPipeline(Pipeline::Stage pipelineStage = {}, PipelineGraphicsCreate pipelineCreate = {});

    static std::shared_ptr<MaterialPipeline> Create(const Pipeline::Stage& pipelineStage, const PipelineGraphicsCreate& pipelineCreate);
    bool                                     BindPipeline(const CommandBuffer& commandBuffer);

    std::type_index GetTypeIndex() const override { return typeid(MaterialPipeline); }

    const Pipeline::Stage&        GetStage() const { return pipelineStage; }
    const PipelineGraphicsCreate& GetPipelineCreate() const { return pipelineCreate; }
    const PipelineGraphics*       GetPipeline() const { return pipeline.get(); }

private:
    Pipeline::Stage                   pipelineStage;
    PipelineGraphicsCreate            pipelineCreate;
    const RenderStage*                renderStage = nullptr;
    std::unique_ptr<PipelineGraphics> pipeline;
};
}   // namespace MapleLeaf