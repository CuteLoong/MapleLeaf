#pragma once

#include "CommandBuffer.hpp"
#include "Shader.hpp"

namespace MapleLeaf {
class Pipeline
{
public:
    /**
     * Represents position in the render structure, first value being the renderpass(renderstage) and second for subpass.
     */
    using Stage = std::pair<uint32_t, uint32_t>;

    Pipeline()          = default;
    virtual ~Pipeline() = default;

    void BindPipeline(const CommandBuffer& commandBuffer) const { vkCmdBindPipeline(commandBuffer, GetPipelineBindPoint(), GetPipeline()); }

    virtual const Shader*                GetShader() const              = 0;
    virtual bool                         IsPushDescriptors() const      = 0;
    virtual const VkDescriptorSetLayout& GetDescriptorSetLayout() const = 0;
    virtual const VkDescriptorPool&      GetDescriptorPool() const      = 0;
    virtual const VkPipeline&            GetPipeline() const            = 0;
    virtual const VkPipelineLayout&      GetPipelineLayout() const      = 0;
    virtual const VkPipelineBindPoint&   GetPipelineBindPoint() const   = 0;
};
}   // namespace MapleLeaf