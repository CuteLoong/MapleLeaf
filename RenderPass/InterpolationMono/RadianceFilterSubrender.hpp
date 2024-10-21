#pragma once

#include "DescriptorHandler.hpp"
#include "Image2d.hpp"
#include "PipelineCompute.hpp"
#include "PushHandler.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MONO_Subrender {
using namespace MapleLeaf;

class RadianceFilterSubrender : public Subrender
{
public:
    explicit RadianceFilterSubrender(const Pipeline::Stage& pipelineStage);

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineRadianceFilter;

    std::vector<DescriptorsHandler> descriptorSetRadianceFilter;
    std::vector<PushHandler>        pushHandlers;

    uint32_t frameID = 0;

    // std::unique_ptr<Image2d>              filteredRadiance;
    std::vector<std::unique_ptr<Image2d>> mipImage;
};
}   // namespace MONO_Subrender