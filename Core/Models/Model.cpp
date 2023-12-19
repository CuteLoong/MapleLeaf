#include "Model.hpp"

namespace MapleLeaf {
Model::Model(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
    : Model()
{
    Initialize(vertices, indices);
    BindInfoToGPU();
    InitBLASInput();
}

void Model::BindInfoToGPU()
{
    if (status != Status::GPU_BINDING) {
        SetVertices(vertices);
        SetIndices(indices);
        status = Status::GPU_BINDING;
    }
}

void Model::Initialize(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
{
    this->vertices.assign(vertices.begin(), vertices.end());
    this->indices.assign(indices.begin(), indices.end());

    vertexCount = static_cast<uint32_t>(vertices.size());
    indexCount  = static_cast<uint32_t>(indices.size());

    status = Status::CPU_BINDING;

    minExtents = glm::vec3(std::numeric_limits<float>::infinity());
    maxExtents = glm::vec3(-std::numeric_limits<float>::infinity());

    for (const auto& vertex : vertices) {
        glm::vec3 position(vertex.position);
        minExtents = glm::vec3(std::min(minExtents.x, position.x), std::min(minExtents.y, position.y), std::min(minExtents.z, position.z));
        maxExtents = glm::vec3(std::max(maxExtents.x, position.x), std::max(maxExtents.y, position.y), std::max(maxExtents.z, position.z));
    }

    radius = glm::length(maxExtents - minExtents) / 2.0;
}

void Model::InitBLASInput()
{
    blasInput = std::make_unique<BLASInput>();

    uint32_t maxPrimitiveCount = indexCount / 3;

    VkAccelerationStructureGeometryTrianglesDataKHR trianglesData = {};
    trianglesData.sType                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    trianglesData.vertexFormat                                    = VK_FORMAT_R32G32B32_SFLOAT;
    trianglesData.vertexData.deviceAddress                        = vertexBuffer->GetDeviceAddress();
    trianglesData.vertexStride                                    = sizeof(Vertex3D);
    trianglesData.indexType                                       = GetIndexType();
    trianglesData.indexData.deviceAddress                         = indexBuffer->GetDeviceAddress();
    trianglesData.maxVertex                                       = vertexCount - 1;

    VkAccelerationStructureGeometryKHR geometry = {};
    geometry.sType                              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType                       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.flags                              = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometry.geometry.triangles                 = trianglesData;

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {};
    buildRangeInfo.primitiveCount                           = maxPrimitiveCount;
    buildRangeInfo.primitiveOffset                          = 0;
    buildRangeInfo.firstVertex                              = 0;
    buildRangeInfo.transformOffset                          = 0;

    blasInput->geometry       = std::move(geometry);
    blasInput->buildRangeInfo = std::move(buildRangeInfo);
}

void Model::SetVertices(const std::vector<Vertex3D>& vertices)
{
    vertexBuffer = nullptr;
    vertexCount  = static_cast<uint32_t>(vertices.size());

    if (vertices.empty()) return;

    Buffer vertexStaging(sizeof(Vertex3D) * vertices.size(),
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertices.data());
    vertexBuffer = std::make_unique<Buffer>(vertexStaging.GetSize(),
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = vertexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, vertexStaging.GetBuffer(), vertexBuffer->GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();
}

void Model::SetIndices(const std::vector<uint32_t>& indices)
{
    indexBuffer = nullptr;
    indexCount  = static_cast<uint32_t>(indices.size());

    if (indices.empty()) return;

    Buffer indexStaging(sizeof(uint32_t) * indices.size(),
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        indices.data());
    indexBuffer = std::make_unique<Buffer>(indexStaging.GetSize(),
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,   // ray tacing flag
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = indexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, indexStaging.GetBuffer(), indexBuffer->GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();
}

bool Model::CmdRender(const CommandBuffer& commandBuffer, uint32_t instances)
{
    BindInfoToGPU();

    if (vertexBuffer && indexBuffer) {
        VkBuffer     vertexBuffers[1] = {vertexBuffer->GetBuffer()};
        VkDeviceSize offsets[1]       = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, GetIndexType());
        vkCmdDrawIndexed(commandBuffer, indexCount, instances, 0, 0, 0);
    }
    else if (vertexBuffer && !indexBuffer) {
        VkBuffer     vertexBuffers[1] = {vertexBuffer->GetBuffer()};
        VkDeviceSize offsets[1]       = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(commandBuffer, vertexCount, instances, 0, 0);
    }
    else {
        return false;
    }

    return true;
}


// std::vector<Vertex3D> Model::GetVertices(std::size_t offset) const
// {
//     Buffer vertexStaging(vertexBuffer->GetSize(),
//                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//     CommandBuffer commandBuffer;

//     VkBufferCopy copyRegion = {};
//     copyRegion.size         = vertexStaging.GetSize();
//     vkCmdCopyBuffer(commandBuffer, vertexBuffer->GetBuffer(), vertexStaging.GetBuffer(), 1, &copyRegion);

//     commandBuffer.SubmitIdle();

//     void* verticesMemory;
//     vertexStaging.MapMemory(&verticesMemory);
//     std::vector<Vertex3D> vertices(vertexCount);

//     auto sizeOfSrcT = vertexStaging.GetSize() / vertexCount;

//     for (uint32_t i = 0; i < vertexCount; i++) {
//         std::memcpy(&vertices[i], static_cast<char*>(verticesMemory) + (i * sizeOfSrcT) + offset, sizeof(Vertex3D));
//     }

//     vertexStaging.UnmapMemory();
//     return vertices;
// }

// std::vector<uint32_t> Model::GetIndices(std::size_t offset) const
// {
//     Buffer indexStaging(indexBuffer->GetSize(),
//                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//     CommandBuffer commandBuffer;

//     VkBufferCopy copyRegion = {};
//     copyRegion.size         = indexStaging.GetSize();
//     vkCmdCopyBuffer(commandBuffer, indexBuffer->GetBuffer(), indexStaging.GetBuffer(), 1, &copyRegion);

//     commandBuffer.SubmitIdle();

//     void* indicesMemory;
//     indexStaging.MapMemory(&indicesMemory);
//     std::vector<uint32_t> indices(indexCount);

//     auto sizeOfSrcT = indexStaging.GetSize() / indexCount;

//     for (uint32_t i = 0; i < indexCount; i++) {
//         std::memcpy(&indices[i], static_cast<char*>(indicesMemory) + (i * sizeOfSrcT) + offset, sizeof(uint32_t));
//     }

//     indexStaging.UnmapMemory();
//     return indices;
// }
}   // namespace MapleLeaf