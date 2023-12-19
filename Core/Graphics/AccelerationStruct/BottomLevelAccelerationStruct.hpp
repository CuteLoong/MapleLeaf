#pragma once

#include "AccelerationStruct.hpp"

namespace MapleLeaf {
class BottomLevelAccelerationStruct : public AccelerationStruct
{
public:
    explicit BottomLevelAccelerationStruct(ASBuildInfo& buildInfo, VkDeviceAddress scratchAddress);

private:
};
}   // namespace MapleLeaf