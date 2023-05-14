#pragma once

#include "RenderStage.hpp"
#include "vector"


namespace MapleLeaf {
class Renderer
{
    friend class Graphics;

public:
    Renderer()          = default;
    virtual ~Renderer() = default;

    virtual void Start()  = 0;
    virtual void Update() = 0;

    void AddRenderStage(std::unique_ptr<RenderStage>&& renderStage) { renderStages.emplace_back(std::move(renderStage)); }

private:
    bool                                      started = false;
    std::vector<std::unique_ptr<RenderStage>> renderStages;
};
}   // namespace MapleLeaf