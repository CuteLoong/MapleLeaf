#pragma once

#include "DescriptorHandler.hpp"
#include "PipelineCompute.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"

namespace MapleLeaf {
class GaussianBlurXYSubrender : public Subrender
{
public:
    struct GaussianData
    {
        int   radius;   // radius less than 7
        float sigma;    // sigma less than radius

        GaussianData(int radius = 3, float sigma = 2.0f)
            : radius(radius)
            , sigma(sigma)
        {}
    };

    explicit GaussianBlurXYSubrender(const Pipeline::Stage& pipelineStage, std::string filterTextureName,
                                     const GaussianData& gaaussianData = GaussianData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineCompute pipelineX;
    PipelineCompute pipelineY;

    UniformHandler     uniformGaussian[2];
    UniformHandler     uniformScene[2];
    StorageHandler     storageWeights[2];
    DescriptorsHandler descriptorSet[2];

    GaussianData       gaussianData;
    std::string        filterTextureName;
    std::vector<float> weights;
};
}   // namespace MapleLeaf