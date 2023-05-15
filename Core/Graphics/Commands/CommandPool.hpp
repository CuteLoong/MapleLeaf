#pragma once

#include "volk.h"
#include "vulkan/vulkan_core.h"
#include <stdint.h>


namespace MapleLeaf {
class CommandPool
{
public:
    explicit CommandPool(const uint32_t id);

    ~CommandPool();

    operator const VkCommandPool&() const { return commandPool; }

    const VkCommandPool& GetCommandPool() const { return commandPool; }
    const uint32_t&      GetPoolId() const { return poolId; }

private:
    uint32_t      poolId;
    VkCommandPool commandPool = VK_NULL_HANDLE;
};
}   // namespace MapleLeaf