#include "Model.hpp"
#include "glm/geometric.hpp"

namespace MapleLeaf {
template<typename T>
Model::Model(const std::vector<T>& vertices, const std::vector<uint32_t>& indices)
    : Model()
{
    Initialize(vertices, indices);
}

template<typename T>
std::vector<T> Model::GetVertices(std::size_t offset) const
{
    Buffer vertexStaging(vertexBuffer->GetSize(),
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = vertexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, vertexBuffer->GetBuffer(), vertexStaging.GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();

    void* verticesMemory;
    vertexStaging.MapMemory(&verticesMemory);
    std::vector<T> vertices(vertexCount);

    auto sizeOfSrcT = vertexStaging.GetSize() / vertexCount;

    for (uint32_t i = 0; i < vertexCount; i++) {
        std::memcpy(&vertices[i], static_cast<char*>(verticesMemory) + (i * sizeOfSrcT) + offset, sizeof(T));
    }

    vertexStaging.UnmapMemory();
    return vertices;
}

std::vector<uint32_t> Model::GetIndices(std::size_t offset) const
{
    Buffer indexStaging(indexBuffer->GetSize(),
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = indexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, indexBuffer->GetBuffer(), indexStaging.GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();

    void* indicesMemory;
    indexStaging.MapMemory(&indicesMemory);
    std::vector<uint32_t> indices(indexCount);

    auto sizeOfSrcT = indexStaging.GetSize() / indexCount;

    for (uint32_t i = 0; i < indexCount; i++) {
        std::memcpy(&indices[i], static_cast<char*>(indicesMemory) + (i * sizeOfSrcT) + offset, sizeof(uint32_t));
    }

    indexStaging.UnmapMemory();
    return indices;
}

template<typename T>
void Model::SetVertices(const std::vector<T>& vertices)
{
    vertexBuffer = nullptr;
    vertexCount  = static_cast<uint32_t>(vertices.size());

    if (vertices.empty()) return;

    Buffer vertexStaging(sizeof(T) * vertices.size(),
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertices.data());
    vertexBuffer = std::make_unique<Buffer>(vertexStaging.GetSize(),
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBuffer commandBuffer;

    VkBufferCopy copyRegion = {};
    copyRegion.size         = indexStaging.GetSize();
    vkCmdCopyBuffer(commandBuffer, indexStaging.GetBuffer(), indexBuffer->GetBuffer(), 1, &copyRegion);

    commandBuffer.SubmitIdle();
}

template<typename T>
void Model::Initialize(const std::vector<T>& vertices, const std::vector<uint32_t>& indices)
{
    SetVertices(vertices);
    SetIndices(indices);

    minExtents = glm::vec3(std::numeric_limits<float>::infinity());
    maxExtents = glm::vec3(-std::numeric_limits<float>::infinity());

    for (const auto& vertex : vertices) {
        glm::vec3 position(vertex.position);
        minExtents = glm::vec3(std::min(minExtents.x, position.x), std::min(minExtents.y, position.y), std::min(minExtents.z, position.z));
        maxExtents = glm::vec3(std::max(minExtents.x, position.x), std::max(minExtents.y, position.y), std::max(minExtents.z, position.z));
    }

    radius = glm::length(maxExtents - minExtents) / 2.0;
}

bool Model::CmdRender(const CommandBuffer& commandBuffer, uint32_t instances) const
{
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
}   // namespace MapleLeaf