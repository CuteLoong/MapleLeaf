#include "ShadowRender.hpp"
#include "Mesh.hpp"
#include "Scenes.hpp"
#include "ShadowSystem.hpp"

namespace MapleLeaf {
ShadowRender::ShadowRender() {}

void ShadowRender::Start() {}

void ShadowRender::Update() {}

bool ShadowRender::CmdRender(const CommandBuffer& commandBuffer, const PipelineGraphics& pipeline)
{
    if (!Scenes::Get()->GetScene()->GetSystem<ShadowSystem>()) return false;

    auto transform = GetEntity()->GetComponent<Transform>();

    if (!transform) return false;

    pushObject.Push(
        "mvp", Scenes::Get()->GetScene()->GetSystem<ShadowSystem>()->GetShadowCascade().GetLightProjectionViewMatrix() * transform->GetWorldMatrix());

    // Gets required components.
    auto mesh = GetEntity()->GetComponent<Mesh>();

    if (!mesh || !mesh->GetModel()) return false;

    // Updates descriptors.
    descriptorSet.Push("PushObject", pushObject);

    if (!descriptorSet.Update(pipeline)) return false;

    // Draws the object.
    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    pushObject.BindPush(commandBuffer, pipeline);
    return mesh->GetModel()->CmdRender(commandBuffer);
}
}   // namespace MapleLeaf