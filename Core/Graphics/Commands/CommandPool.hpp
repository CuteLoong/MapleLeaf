#pragma once

#include "volk.h"
#include <thread>

namespace MapleLeaf {
class CommandPool
{
public:
    explicit CommandPool(const std::thread::id& threadId = std::this_thread::get_id());

    ~CommandPool();

    operator const VkCommandPool&() const { return commandPool; }

    const VkCommandPool&   GetCommandPool() const { return commandPool; }
    const std::thread::id& GetPoolId() const { return threadId; }

private:
    std::thread::id threadId;
    VkCommandPool   commandPool = VK_NULL_HANDLE;
};
}   // namespace MapleLeaf