#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class WarpRenderer : public Renderer
{
public:
    WarpRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test