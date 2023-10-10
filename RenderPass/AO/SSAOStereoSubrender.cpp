#include "SSAOStereoSubrender.hpp"
#include "Devices.hpp"
#include "Log.hpp"

namespace MapleLeaf {
SSAOStereoSubrender::SSAOStereoSubrender(const Pipeline::Stage& pipelineStage, SSAOData ssaoData)
    : Subrender(pipelineStage)
    , ssaoData(ssaoData)
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/AO/SSAOStereo.vert", "Shader/AO/SSAOStereo.frag"}, {}, {GetDefines()},
                                PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None))
{

    noiseSize = Devices::Get()->GetWindow()->GetSize() / ssaoData.noiseScale;
    noise     = (Resources::Get()->GetThreadPool().Enqueue(ComputeNoise, noiseSize.x, noiseSize.y));

    sampleKernel.resize(ssaoData.kernelSize);
    for (uint32_t i = 0; i < ssaoData.kernelSize; i++) {
        glm::vec3 p = Maths::hammersleyUniform(i, ssaoData.kernelSize);

        sampleKernel[i] = glm::vec4(p, 0.0f);
        // Skew sample point distance on a curve so more cluster around the origin
        float dist = (float)i / (float)ssaoData.kernelSize;
        dist       = glm::mix(0.1f, 1.0f, dist * dist);
        sampleKernel[i] *= dist;
    }
}

void SSAOStereoSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("kernel", *sampleKernel.data(), sizeof(glm::vec4) * ssaoData.kernelSize);
    uniformScene.Push("projection", camera->GetStereoProjectionMatrix());
    uniformScene.Push("view", camera->GetStereoViewMatrix());
    uniformScene.Push("cameraPosition", camera->GetStereoViewPosition());

    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));
    descriptorSet.Push("samplerNoise", *noise);

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the object.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void SSAOStereoSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::shared_ptr<Image2d> SSAOStereoSubrender::ComputeNoise(uint32_t width, uint32_t height)
{
    std::vector<glm::vec4> ssaoNoise(width * height);

    for (uint32_t i = 0; i < width * height; i++) {
        glm::vec2 dir = glm::normalize(glm::linearRand(glm::vec2(-1.0f), glm::vec2(1.0f))) * 0.5f + 0.5f;
        ssaoNoise[i]  = glm::vec4(dir, 0.0f, 0.0f);
    }

    std::unique_ptr<uint8_t[]> dataPtr(new uint8_t[width * height * 4]);
    for (int i = 0; i < width * height; ++i) {
        dataPtr[i * 4 + 0] = static_cast<uint8_t>(ssaoNoise[i].x * 255);   // R
        dataPtr[i * 4 + 1] = static_cast<uint8_t>(ssaoNoise[i].y * 255);   // G
        dataPtr[i * 4 + 2] = static_cast<uint8_t>(ssaoNoise[i].z * 255);   // B
        dataPtr[i * 4 + 3] = static_cast<uint8_t>(ssaoNoise[i].w * 255);   // A
    }

    std::shared_ptr<Image2d> noiseImage = std::make_shared<Image2d>(std::make_unique<Bitmap>(std::move(dataPtr), glm::uvec2(width, height)),
                                                                    VK_FORMAT_R8G8B8A8_UNORM,
                                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                                    VK_FILTER_NEAREST);

    return noiseImage;
}

std::vector<Shader::Define> SSAOStereoSubrender::GetDefines() const
{
    return {{"SSAO_KERNEL_SIZE", std::to_string(16)}, {"SSAO_RADIUS", std::to_string(0.1)}};
}
}   // namespace MapleLeaf