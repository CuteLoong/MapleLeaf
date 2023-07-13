#pragma once

#include "glm/glm.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MapleLeaf {
using NodeID = int32_t;

struct SceneNode
{
public:
    std::string name;
    glm::mat4   transform;
    NodeID      parent;
    std::vector<SceneNode> children;
};

class SceneGraph
{
public:

private:
    
};
}   // namespace MapleLeaf