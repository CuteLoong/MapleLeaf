#pragma once

#include "CommandBuffer.hpp"
#include "Pipeline.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
class PipelineCompute : public Pipeline
{
public:
    explicit PipelineCompute(std::filesystem::path shaderStage, std::vector<Shader::Define> defines = {}, bool pushDescriptors = false);
    ~PipelineCompute();

    void CmdRender(const CommandBuffer& commandBuffer, const glm::uvec2& extent) const;

private:
    void CreateShaderProgram();
    void CreateDescriptorLayout();
    void CreateDescriptorPool();
    void CreatePipelineLayout();
    void CreatePipelineCompute();

    std::filesystem::path       shaderStage;
    std::vector<Shader::Define> defines;
    bool                        pushDescriptors;

    std::unique_ptr<Shader> shader;

    VkShaderModule                  shaderModule          = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};

    std::map<uint32_t, VkDescriptorSetLayout> descriptorSetLayouts;
    VkDescriptorPool                          descriptorPool = VK_NULL_HANDLE;

    VkPipeline          pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint pipelineBindPoint;
};
}   // namespace MapleLeaf