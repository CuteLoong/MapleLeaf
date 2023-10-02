#pragma once

#include "PipelineGraphics.hpp"
#include "Subrender.hpp"
#include "UniformHandler.hpp"


namespace MapleLeaf {
class MeshesSubrender : public Subrender
{
public:
    enum class Sort
    {
        None,
        Front,
        Back
    };

    explicit MeshesSubrender(const Pipeline::Stage& pipelineStage, Sort sort = Sort::None);

    void Render(const CommandBuffer& commandBuffer) override;
    void PostRender(const CommandBuffer& commandBuffer) override;

private:
    Sort           sort;
    UniformHandler uniformScene;
};
}   // namespace MapleLeaf