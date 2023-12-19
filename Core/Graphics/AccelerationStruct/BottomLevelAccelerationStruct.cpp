#include "BottomLevelAccelerationStruct.hpp"

#include "Graphics.hpp"

namespace MapleLeaf {
BottomLevelAccelerationStruct::BottomLevelAccelerationStruct(ASBuildInfo& buildInfo, VkDeviceAddress scratchAddress)
{
    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {};
    accelerationStructureCreateInfo.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationStructureCreateInfo.size                                 = buildInfo.buildSizesInfo.accelerationStructureSize;

    buffer = std::make_unique<Buffer>(accelerationStructureCreateInfo.size,
                                      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    accelerationStructureCreateInfo.buffer = buffer->GetBuffer();

    Graphics::CheckVk(
        vkCreateAccelerationStructureKHR(*Graphics::Get()->GetLogicalDevice(), &accelerationStructureCreateInfo, nullptr, &accelerationStruct));

    buildInfo.buildGeometryInfo.dstAccelerationStructure  = accelerationStruct;
    buildInfo.buildGeometryInfo.scratchData.deviceAddress = scratchAddress;

    CommandBuffer commandBuffer;

    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo.buildGeometryInfo, &buildInfo.buildRangeInfo);

    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    memoryBarrier.dstAccessMask   = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                         VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                         0,
                         1,
                         &memoryBarrier,
                         0,
                         nullptr,
                         0,
                         nullptr);

    commandBuffer.SubmitIdle();
}
}   // namespace MapleLeaf