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

    bool                                             IsPushDescriptors() const override { return pushDescriptors; }
    const Shader*                                    GetShader() const override { return shader.get(); }
    const std::map<uint32_t, VkDescriptorSetLayout>& GetBindlessDescriptorSetLayouts() const override { return descriptorSetBindlessLayouts; }
    const std::map<uint32_t, VkDescriptorSetLayout>& GetNormalDescriptorSetLayouts() const override { return descriptorSetNormalLayouts; }
    const VkDescriptorPool&                          GetDescriptorPool() const override { return descriptorPool; }
    const VkPipeline&                                GetPipeline() const override { return pipeline; }
    const VkPipelineLayout&                          GetPipelineLayout() const override { return pipelineLayout; }
    const VkPipelineBindPoint&                       GetPipelineBindPoint() const override { return pipelineBindPoint; }

private:
    void CreateShaderProgram();
    void CreateDescriptorLayout();
    void CreateDescriptorPool();
    void CreatePipelineLayout();
    void CreatePipelineCompute();
    void CreateBindlessDescriptorLayout(uint32_t setIndex, const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
    void CreateNormalDescriptorLayout(uint32_t setIndex, const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);

    std::filesystem::path       shaderStage;
    std::vector<Shader::Define> defines;
    bool                        pushDescriptors;

    std::unique_ptr<Shader> shader;

    VkShaderModule                  shaderModule          = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};

    std::map<uint32_t, VkDescriptorSetLayout> descriptorSetBindlessLayouts;
    std::map<uint32_t, VkDescriptorSetLayout> descriptorSetNormalLayouts;
    VkDescriptorPool                          descriptorPool = VK_NULL_HANDLE;

    VkPipeline          pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint pipelineBindPoint;
};
}   // namespace MapleLeaf