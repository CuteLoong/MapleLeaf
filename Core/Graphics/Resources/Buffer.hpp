#pragma once

#include "CommandBuffer.hpp"
#include "volk.h"

namespace MapleLeaf {
class Buffer
{
public:
    enum class Status
    {
        Reset,
        Changed,
        Normal
    };

    Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data = nullptr);
    virtual ~Buffer();

    void MapMemory(void** data) const;
    void UnmapMemory() const;

    VkDeviceSize          GetSize() const { return size; }
    const VkBuffer&       GetBuffer() const { return buffer; }
    const VkDeviceMemory& GetBufferMemory() const { return bufferMemory; }
    VkDeviceAddress       GetDeviceAddress() const { return deviceAddress; }

    static uint32_t FindMemoryType(uint32_t typeFilter, const VkMemoryPropertyFlags& requiredProperties);
    static void     InsertBufferMemoryBarrier(const CommandBuffer& commandBuffer, const VkBuffer& buffer, VkAccessFlags srcAccessMask,
                                              VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

protected:
    VkDeviceSize    size;
    VkDeviceAddress deviceAddress = 0;
    VkBuffer        buffer        = VK_NULL_HANDLE;
    VkDeviceMemory  bufferMemory  = VK_NULL_HANDLE;
};
}   // namespace MapleLeaf