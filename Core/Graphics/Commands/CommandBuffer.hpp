#pragma once

#include "CommandPool.hpp"
#include <memory>

namespace MapleLeaf {
class CommandBuffer
{
public:
    explicit CommandBuffer(bool begin = true, VkQueueFlagBits queueType = VK_QUEUE_GRAPHICS_BIT,
                           VkCommandBufferLevel bufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    ~CommandBuffer();

    void Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void End();

    void SubmitIdle();
    void Submit(const VkSemaphore& waitSemaphore = VK_NULL_HANDLE, const VkSemaphore& signalSemaphore = VK_NULL_HANDLE,
                VkFence fence = VK_NULL_HANDLE);

    operator const VkCommandBuffer&() const { return commandBuffer; }

    const VkCommandBuffer& GetCommandBuffer() const { return commandBuffer; }
    bool                   IsRunning() const { return running; }

private:
    VkQueue GetQueue() const;

    std::shared_ptr<CommandPool> commandPool;

    VkQueueFlagBits queueType;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    bool            running       = false;
};
}   // namespace MapleLeaf