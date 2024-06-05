#include "ASScene.hpp"

#include "BottomLevelAccelerationStruct.hpp"
#include "Model.hpp"
#include "Resources.hpp"
#include "Scenes.hpp"
#include "TopLevelAccelerationStruct.hpp"
#include "vulkan/vulkan_core.h"

namespace MapleLeaf {
ASScene::ASScene() {}

ASScene::~ASScene() {}

inline VkTransformMatrixKHR ASScene::ToTransformMatrixKHR(glm::mat4 matrix)
{
    glm::mat4            temp = glm::transpose(matrix);
    VkTransformMatrixKHR transformMatrixKHR{};
    memcpy(&transformMatrixKHR, &temp, sizeof(transformMatrixKHR));
    return transformMatrixKHR;
}

void ASScene::Start()
{
    BuildBLAS();
    BuildTLAS();
}

void ASScene::Update()
{
    // BuildBLAS(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR, true);
    BuildTLAS(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR, true);
}

void ASScene::BuildBLAS(VkBuildAccelerationStructureFlagsKHR flags, bool update)
{
    const auto&  models      = Resources::Get()->FindAll<Model>();
    VkDeviceSize ASTotalSize = 0;
    VkDeviceSize scratchSize = 0;

    std::vector<ASBuildInfo> ASBuildInfos(models.size());

    for (size_t i = 0; i < models.size(); ++i) {
        const auto& model = models[i];

        VkAccelerationStructureBuildGeometryInfoKHR geometryInfo{};
        geometryInfo.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geometryInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        geometryInfo.mode          = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geometryInfo.flags         = model->GetBLASInput()->flags | flags;
        geometryInfo.geometryCount = 1;
        geometryInfo.pGeometries   = &model->GetBLASInput()->geometry;

        VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
        buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        ASBuildInfos[i] = std::move(ASBuildInfo{geometryInfo, buildSizesInfo, model->GetBLASInput()->buildRangeInfo});

        AccelerationStruct::GetAccelerationStructureBuildSizes(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                               &ASBuildInfos[i].buildGeometryInfo,
                                                               &ASBuildInfos[i].buildRangeInfo.primitiveCount,
                                                               &ASBuildInfos[i].buildSizesInfo);

        ASTotalSize += ASBuildInfos[i].buildSizesInfo.accelerationStructureSize;
        scratchSize = std::max(ASBuildInfos[i].buildSizesInfo.buildScratchSize, scratchSize);
    }

    Buffer scratchBuffer(
        scratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    bottomLevelaccelerationStructs.reserve(models.size());

    for (size_t i = 0; i < models.size(); ++i) {
        bottomLevelaccelerationStructs.emplace_back(
            std::make_unique<BottomLevelAccelerationStruct>(ASBuildInfos[i], scratchBuffer.GetDeviceAddress()));
    }
}

void ASScene::BuildTLAS(VkBuildAccelerationStructureFlagsKHR flags, bool update)
{
    const auto& meshes = Scenes::Get()->GetScene()->GetComponents<Mesh>();

    std::vector<VkAccelerationStructureInstanceKHR> instances{};
    instances.reserve(meshes.size());

    for (uint32_t i = 0; i < meshes.size(); ++i) {
        const auto& instance   = meshes[i];
        uint32_t    modelIndex = Resources::Get()->GetResourceIndex(instance->GetModel());

        VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
        accelerationStructureInstance.transform           = ToTransformMatrixKHR(instance->GetEntity()->GetComponent<Transform>()->GetWorldMatrix());
        accelerationStructureInstance.instanceCustomIndex = i;
        accelerationStructureInstance.mask                = 0xFF;
        accelerationStructureInstance.instanceShaderBindingTableRecordOffset = 0;
        accelerationStructureInstance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        accelerationStructureInstance.accelerationStructureReference         = bottomLevelaccelerationStructs[modelIndex]->GetDeviceAddress();

        instances.emplace_back(accelerationStructureInstance);
    }

    Buffer instancesBuffer(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(),
                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           instances.data());

    VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
    instancesData.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instancesData.data.deviceAddress = instancesBuffer.GetDeviceAddress();

    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances = instancesData;

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildGeometryInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.flags                    = flags;
    buildGeometryInfo.geometryCount            = 1;
    buildGeometryInfo.pGeometries              = &geometry;
    buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;

    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount  = static_cast<uint32_t>(instances.size());
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex     = 0;
    buildRangeInfo.transformOffset = 0;

    ASBuildInfo buildInfo{buildGeometryInfo, buildSizesInfo, buildRangeInfo};

    AccelerationStruct::GetAccelerationStructureBuildSizes(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                           &buildInfo.buildGeometryInfo,
                                                           &buildInfo.buildRangeInfo.primitiveCount,
                                                           &buildInfo.buildSizesInfo);

    topLevelAccelerationStruct = std::make_unique<TopLevelAccelerationStruct>(buildInfo);
}
}   // namespace MapleLeaf