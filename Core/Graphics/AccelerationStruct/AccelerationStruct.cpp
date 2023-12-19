#include "AccelerationStruct.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
AccelerationStruct::AccelerationStruct() {}

AccelerationStruct::~AccelerationStruct()
{
    DestroyAccelerationStruct();
}

void AccelerationStruct::DestroyAccelerationStruct()
{
    if (accelerationStruct != VK_NULL_HANDLE) {
        vkDestroyAccelerationStructureKHR(*Graphics::Get()->GetLogicalDevice(), accelerationStruct, nullptr);
        accelerationStruct = VK_NULL_HANDLE;
    }
}

VkDeviceAddress AccelerationStruct::GetDeviceAddress() const
{
    VkAccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddressInfo{};
    accelerationStructureDeviceAddressInfo.sType                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationStructureDeviceAddressInfo.accelerationStructure = accelerationStruct;

    return vkGetAccelerationStructureDeviceAddressKHR(*Graphics::Get()->GetLogicalDevice(), &accelerationStructureDeviceAddressInfo);
}

void AccelerationStruct::GetAccelerationStructureBuildSizes(VkAccelerationStructureBuildTypeKHR                buildType,
                                                            const VkAccelerationStructureBuildGeometryInfoKHR& buildGeometryInfo,
                                                            const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)
{
    vkGetAccelerationStructureBuildSizesKHR(*Graphics::Get()->GetLogicalDevice(), buildType, &buildGeometryInfo, pMaxPrimitiveCounts, pSizeInfo);
}

WriteDescriptorSet AccelerationStruct::GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                                          const std::optional<OffsetSize>& offsetSize) const
{
    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructInfo = {};
    accelerationStructInfo.sType                                        = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationStructInfo.accelerationStructureCount                   = 1;
    accelerationStructInfo.pAccelerationStructures                      = &accelerationStruct;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet               = VK_NULL_HANDLE;   // Will be set in the descriptor handler.
    descriptorWrite.dstBinding           = binding;
    descriptorWrite.dstArrayElement      = 0;
    descriptorWrite.descriptorCount      = 1;
    descriptorWrite.descriptorType       = descriptorType;

    return {descriptorWrite, accelerationStructInfo};
}

VkDescriptorSetLayoutBinding AccelerationStruct::GetDescriptorSetLayout(uint32_t binding, VkShaderStageFlags stage, uint32_t count)
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding                      = binding;
    descriptorSetLayoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    descriptorSetLayoutBinding.descriptorCount              = count;
    descriptorSetLayoutBinding.stageFlags                   = stage;
    descriptorSetLayoutBinding.pImmutableSamplers           = nullptr;
    return descriptorSetLayoutBinding;
}
}   // namespace MapleLeaf