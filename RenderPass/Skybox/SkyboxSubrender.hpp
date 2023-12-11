#pragma once

#include "DescriptorHandler.hpp"
#include "Future.hpp"
#include "PipelineCompute.hpp"
#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"


namespace MapleLeaf {
class SkyboxSubrender : public Subrender
{
public:
    explicit SkyboxSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipelineGraphics;

    UniformHandler uniformCamera;
    UniformHandler uniformSkybox;

    DescriptorsHandler descriptorSet;

    Future<std::unique_ptr<Image2d>> brdf;
    static std::unique_ptr<Image2d>  ComputeBRDF(uint32_t size);
};
}   // namespace MapleLeaf