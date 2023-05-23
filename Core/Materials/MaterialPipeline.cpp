#include "MaterialPipeline.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
MaterialPipeline::MaterialPipeline(Pipeline::Stage pipelineStage, PipelineGraphicsCreate pipelineCreate)
    : pipelineStage(std::move(pipelineStage))
    , pipelineCreate(std::move(pipelineCreate))
{}

std::shared_ptr<MaterialPipeline> MaterialPipeline::Create(const Pipeline::Stage& pipelineStage, const PipelineGraphicsCreate& pipelineCreate)
{
    return std::make_shared<MaterialPipeline>(pipelineStage, pipelineCreate);
}

bool MaterialPipeline::BindPipeline(const CommandBuffer& commandBuffer)
{
    auto renderStage = Graphics::Get()->GetRenderStage(pipelineStage.first);

    if (!renderStage) return false;

    if (this->renderStage != renderStage) {
        this->renderStage = renderStage;
        pipeline.reset(pipelineCreate.Create(pipelineStage)); 
    }

    pipeline->BindPipeline(commandBuffer);
    return true;
}
}   // namespace MapleLeaf