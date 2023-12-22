#pragma once

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
class PipelineRayTracing : public Pipeline
{
public:
    explicit PipelineRayTracing(std::vector<std::filesystem::path> shaderStage, std::vector<Shader::Define> defines = {},
                                bool pushDescriptors = false);
    ~PipelineRayTracing();

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
    void CreatePipelineRayTracing();
    void CreateShaderBindingTable();
    void CreateBindlessDescriptorLayout(uint32_t setIndex, const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
    void CreateNormalDescriptorLayout(uint32_t setIndex, const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);

    std::vector<std::filesystem::path> shaderStages;
    std::vector<Shader::Define>        defines;
    uint32_t                           missShaderCount     = 0;
    uint32_t                           hitShaderCount      = 0;
    uint32_t                           callableShaderCount = 0;

    bool pushDescriptors;

    std::unique_ptr<Shader> shader;

    std::vector<VkShaderModule>                       modules;
    std::vector<VkPipelineShaderStageCreateInfo>      stages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

    std::unique_ptr<Buffer>         SBTBuffer;
    VkStridedDeviceAddressRegionKHR rgenRegion;
    VkStridedDeviceAddressRegionKHR missRegion;
    VkStridedDeviceAddressRegionKHR hitRegion;
    VkStridedDeviceAddressRegionKHR callRegion;

    std::map<uint32_t, VkDescriptorSetLayout> descriptorSetBindlessLayouts;
    std::map<uint32_t, VkDescriptorSetLayout> descriptorSetNormalLayouts;
    VkDescriptorPool                          descriptorPool = VK_NULL_HANDLE;

    VkPipeline          pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint pipelineBindPoint;
};
}   // namespace MapleLeaf