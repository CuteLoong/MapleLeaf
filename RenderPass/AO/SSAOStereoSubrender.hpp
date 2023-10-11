#pragma once

#include "Future.hpp"
#include "Image2d.hpp"
#include "Maths.hpp"
#include "PipelineGraphics.hpp"
#include "Resources.hpp"
#include "Scenes.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"
#include "glm/gtc/random.hpp"

namespace MapleLeaf {
class SSAOStereoSubrender : public Subrender
{
public:
    struct SSAOData
    {
        glm::uvec2 noiseScale;
        float      radius;
        uint32_t   kernelSize;

        SSAOData(glm::uvec2 noiseScale = glm::uvec2{1.0f, 1.0f}, float radius = 0.1f, uint32_t kernelSize = 16)
            : noiseScale(noiseScale)
            , radius(radius)
            , kernelSize(kernelSize)
        {}
    };

    explicit SSAOStereoSubrender(const Pipeline::Stage& pipelineStage, SSAOData ssaoData = SSAOData());

    void PreRender(const CommandBuffer& commandBuffer) override;
    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    PipelineGraphics pipeline;

    UniformHandler     uniformScene;
    DescriptorsHandler descriptorSet;

    SSAOData               ssaoData;
    std::vector<glm::vec4> sampleKernel;

    glm::uvec2                       noiseSize;
    Future<std::shared_ptr<Image2d>> noise;

    std::vector<Shader::Define>     GetDefines() const;
    static std::shared_ptr<Image2d> ComputeNoise(uint32_t width, uint32_t height);
};
}   // namespace MapleLeaf