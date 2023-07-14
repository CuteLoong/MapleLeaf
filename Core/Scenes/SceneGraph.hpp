#pragma once

#include "glm/glm.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MapleLeaf {
// using NodeID = int32_t;

class NodeID
{
public:
    NodeID() { index = -1; }
    NodeID(uint32_t index)
        : index(index)
    {}

    int32_t get() const { return index; }
    bool    isValid() const { return index != -1; }

    operator int32_t() const { return index; }
    NodeID operator=(const NodeID rhs)
    {
        index = rhs;
        return *this;
    }

    static constexpr int32_t kInvalidID = -1;
    static NodeID            Invalid() { return NodeID(kInvalidID); }

private:
    int32_t index;
};

struct SceneNode
{
public:
    std::string         name;
    glm::mat4           transform;
    NodeID              parent;
    std::vector<NodeID> children;
    std::vector<uint32_t> meshes;
};

using SceneGraph = std::vector<SceneNode>;
}   // namespace MapleLeaf