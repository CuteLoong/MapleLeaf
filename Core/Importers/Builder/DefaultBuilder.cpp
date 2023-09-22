#include "DefaultBuilder.hpp"

namespace MapleLeaf {

NodeID Builder::AddSceneNode(SceneNode&& node)
{
    if (node.parent.isValid() && node.parent >= sceneGraph.size()) Log::Error("Scene node parent is out of range");
    if (sceneGraph.size() >= std::numeric_limits<int32_t>::max()) Log::Error("Scene graph is too large");

    NodeID newNodeID{static_cast<uint32_t>(sceneGraph.size())};
    sceneGraph.push_back(std::move(node));
    SceneNode& n = sceneGraph.back();
    if (n.parent.isValid()) {
        sceneGraph[n.parent].children.push_back(newNodeID);
        n.transform->SetParent(sceneGraph[n.parent].transform);
    }

    return newNodeID;
}

void Builder::AddLight(std::unique_ptr<Light>&& light)
{
    assert(light != nullptr);
    lights.push_back(std::move(light));
}

void Builder::AddCamera(std::unique_ptr<Camera>&& camera)
{
    assert(camera != nullptr);
    cameras.push_back(std::move(camera));
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