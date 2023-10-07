#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class GPURenderer : public Renderer
{
public:
    GPURenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test