#include "SubrenderHolder.hpp"

namespace MapleLeaf {
void SubrenderHolder::Clear()
{
    stages.clear();
}

void SubrenderHolder::RemoveSubrenderStage(const TypeId& id)
{
    for (auto it = stages.begin(); it != stages.end();) {
        if (it->second == id) {
            it = stages.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SubrenderHolder::PreRenderStage(const Pipeline::Stage& stage, const CommandBuffer& commandBuffer)
{
    for (const auto& [stageIndex, typeId] : stages) {
        if (stageIndex.first != stage) {
            continue;
        }

        if (auto& subrender = subrenders[typeId]) {
            if (subrender->IsEnabled()) {
                subrender->PreRender(commandBuffer);
            }
        }
    }
}

void SubrenderHolder::RenderStage(const Pipeline::Stage& stage, const CommandBuffer& commandBuffer)
{
    for (const auto& [stageIndex, typeId] : stages) {
        if (stageIndex.first != stage) {
            continue;
        }

        if (auto& subrender = subrenders[typeId]) {
            if (subrender->IsEnabled()) {
                subrender->Render(commandBuffer);
            }
        }
    }
}

void SubrenderHolder::PostRenderStage(const Pipeline::Stage& stage, const CommandBuffer& commandBuffer)
{
    for (const auto& [stageIndex, typeId] : stages) {
        if (stageIndex.first != stage) {
            continue;
        }

        if (auto& subrender = subrenders[typeId]) {
            if (subrender->IsEnabled()) {
                subrender->PostRender(commandBuffer);
            }
        }
    }
}
}   // namespace MapleLeaf