#pragma once

#include "Buffer.hpp"
#include "Descriptor.hpp"
#include "Model.hpp"
#include "volk.h"
#include "vulkan/vulkan_core.h"
#include <vector>

namespace MapleLeaf {
struct ASBuildInfo
{
    VkAccelerationStructureBuildGeometryInfoKHR     buildGeometryInfo{};
    VkAccelerationStructureBuildSizesInfoKHR        buildSizesInfo{};
    const VkAccelerationStructureBuildRangeInfoKHR* buildRangeInfo;
};

class AccelerationStruct : public Descriptor
{
    friend class Graphics;

public:
    AccelerationStruct();
    ~AccelerationStruct();

    VkDeviceAddress GetDeviceAddress() const;

    WriteDescriptorSet                  GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                                           const std::optional<OffsetSize>& offsetSize) const override;
    static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, VkShaderStageFlags stage, uint32_t count);

    const VkAccelerationStructureKHR& GetAccelerationStruct() const { return accelerationStruct; }
    static void                       GetAccelerationStructureBuildSizes(VkAccelerationStructureBuildTypeKHR                buildType,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR& buildGeometryInfo,
                                                                         const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo);


    void DestroyAccelerationStruct();

protected:
    VkAccelerationStructureKHR accelerationStruct = VK_NULL_HANDLE;
    std::unique_ptr<Buffer>    buffer;
};
}   // namespace MapleLeaf