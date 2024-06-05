#pragma once

#include "Transform.hpp"
#include "glm/glm.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MapleLeaf {
// using NodeID = int32_t;

class NodeID
{
public:
    struct NodeIDHash
    {
        std::size_t operator()(const NodeID& nodeID) const { return std::hash<int32_t>()(nodeID.get()); }
    };

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

    friend bool operator==(const NodeID& lhs, const NodeID& rhs) { return lhs.index == rhs.index; }

private:
    int32_t index;
};

class SceneNode
{
public:
    SceneNode() = default;

    std::string           name;
    Transform*            transform;   // auto free by unique_ptr in component
    NodeID                parent;
    std::vector<NodeID>   children;
    std::vector<uint32_t> meshes;
};

using SceneGraph = std::vector<SceneNode>;
}   // namespace MapleLeaf