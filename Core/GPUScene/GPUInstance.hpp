#pragma once

#include "Mesh.hpp"

namespace MapleLeaf {
class GPUInstance
{
    friend class GPUScene;

public:
    enum class Status
    {
        None = 0,
        ModelChanged = 1,
        MatrixChanged = 2
    };

    GPUInstance() = default;

    explicit GPUInstance(Mesh* mesh, uint32_t instanceID, uint32_t materialID);

    void   Update();
    Status GetInstanceStatus() const { return instanceStatus; }

private:
    Mesh*  mesh;   // relevent mesh
    std::shared_ptr<Model> model;
    Status                 instanceStatus;

    glm::mat4 modelMatrix;
    glm::vec3 AABBLocalMin;
    uint32_t  indexCount;
    glm::vec3 AABBLocalMax;
    uint32_t  indexOffset;
    uint32_t  vertexCount;
    uint32_t  vertexOffset;
    uint32_t  instanceID;
    uint32_t  materialID;

    // offset first is indexOffset, offset second is vertexOffset
    static std::unordered_map<std::shared_ptr<Model>, std::pair<uint32_t, uint32_t>> modelOffset;
    static std::vector<Vertex3D>                                                     verticesArray;
    static std::vector<uint32_t>                                                     indicesArray;
};
}   // namespace MapleLeaf