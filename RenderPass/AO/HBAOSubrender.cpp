#include "HBAOSubrender.hpp"

#include "Devices.hpp"
#include "Maths.hpp"
#include "Resources.hpp"
#include "Scenes.hpp"
#include "glm/gtc/random.hpp"

namespace MapleLeaf {
HBAOSubrender::HBAOSubrender(const Pipeline::Stage& pipelineStage, HBAOData hbaoData)
    : Subrender(pipelineStage)
    , hbaoData(hbaoData)
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/AO/HBAO.vert", "Shader/AO/HBAO.frag"}, {}, {}, PipelineGraphics::Mode::Polygon,
                                PipelineGraphics::Depth::None))
{
    noiseSize = Devices::Get()->GetWindow()->GetSize() / hbaoData.noiseScale;
    hbaoNoise = (Resources::Get()->GetThreadPool().Enqueue(ComputeNoise, noiseSize.x, noiseSize.y, hbaoData.numRays));
}

void HBAOSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void HBAOSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    float pixelRadius = camera->GetPixelHeight() * hbaoData.sampleRadius / (2.0f * glm::tan(camera->GetFieldOfView() / 2.0f));

    uniformHBAOData.Push("noiseScale", hbaoData.noiseScale);
    uniformHBAOData.Push("numRays", hbaoData.numRays);
    uniformHBAOData.Push("stepCount", hbaoData.stepCount);
    uniformHBAOData.Push("maxRadiusPixels", hbaoData.maxRadiusPixels);
    uniformHBAOData.Push("sampleRadius", hbaoData.sampleRadius);
    uniformHBAOData.Push("pixelRadius", pixelRadius);
    uniformHBAOData.Push("intensity", hbaoData.intensity);
    uniformHBAOData.Push("angleBias", hbaoData.angleBias);

    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("UniformHBAOData", uniformHBAOData);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("hbaoNoise", *hbaoNoise);

    if (!descriptorSet.Update(pipeline)) return;

    pipeline.BindPipeline(commandBuffer);

    // Draws the object.
    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void HBAOSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::shared_ptr<Image2d> HBAOSubrender::ComputeNoise(uint32_t width, uint32_t height, uint32_t numRays)
{
    std::vector<glm::vec4> hbaoNoise(width * height);

    for (uint32_t i = 0; i < width * height; i++) {
        glm::vec3 rand  = glm::normalize(glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f))) * 0.5f + 0.5f;
        float     angle = 2.0f * M_PI * rand.x / numRays;
        hbaoNoise[i]    = glm::vec4(std::cos(angle), std::sin(angle), rand.y, rand.z);
    }

    std::unique_ptr<uint8_t[]> dataPtr(new uint8_t[width * height * 4]);
    for (int i = 0; i < width * height; ++i) {
        dataPtr[i * 4 + 0] = static_cast<uint8_t>(hbaoNoise[i].x * 255);   // R
        dataPtr[i * 4 + 1] = static_cast<uint8_t>(hbaoNoise[i].y * 255);   // G
        dataPtr[i * 4 + 2] = static_cast<uint8_t>(hbaoNoise[i].z * 255);   // B
        dataPtr[i * 4 + 3] = static_cast<uint8_t>(hbaoNoise[i].w * 255);   // A
    }

    std::shared_ptr<Image2d> noiseImage = std::make_shared<Image2d>(std::make_unique<Bitmap>(std::move(dataPtr), glm::uvec2(width, height)),
                                                                    VK_FORMAT_R8G8B8A8_UNORM,
                                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                                    VK_FILTER_NEAREST);

    return noiseImage;
}
}   // namespace MapleLeaf