#include "DefaultBuilder.hpp"

namespace MapleLeaf {
NodeID Builder::AddSceneNode(SceneNode&& node)
{
    if (node.parent.isValid() && node.parent >= sceneGraph.size()) Log::Error("Scene node parent is out of range");
    if (sceneGraph.size() >= std::numeric_limits<int32_t>::max()) Log::Error("Scene graph is too large");

    NodeID newNodeID{static_cast<uint32_t>(sceneGraph.size())};
    sceneGraph.push_back(std::move(node));
    if (node.parent.isValid()) sceneGraph[node.parent].children.push_back(newNodeID);

    return newNodeID;
}



Mesh* Builder::GetMesh(const uint32_t index)
{
    if (index >= meshes.size()) {
        Log::Error("Model index upper than record models!");
        return nullptr;
    }
    return (meshes[index].get());
}
}   // namespace MapleLeaf