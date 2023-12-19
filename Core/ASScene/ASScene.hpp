#pragma once

#include "AccelerationStruct.hpp"

namespace MapleLeaf {
class ASScene
{
    friend class Scene;

public:
    ASScene();
    ~ASScene();

    void Start();
    void Update();

    void BuildBLAS(VkBuildAccelerationStructureFlagBitsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
    void BuildTLAS(VkBuildAccelerationStructureFlagBitsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

    const std::vector<std::unique_ptr<AccelerationStruct>>& GetBottomLevelAccelerationStructs() const { return bottomLevelaccelerationStructs; }
    const std::unique_ptr<AccelerationStruct>&              GetBottomLevelAccelerationStruct(uint32_t index) const
    {
        return bottomLevelaccelerationStructs[index];
    }
    const std::unique_ptr<AccelerationStruct>& GetTopLevelAccelerationStruct() const { return topLevelAccelerationStruct; }

    inline VkTransformMatrixKHR ToTransformMatrixKHR(glm::mat4 matrix);

private:
    std::vector<std::unique_ptr<AccelerationStruct>> bottomLevelaccelerationStructs;
    std::unique_ptr<AccelerationStruct>              topLevelAccelerationStruct;

    bool started = false;
};
}   // namespace MapleLeaf