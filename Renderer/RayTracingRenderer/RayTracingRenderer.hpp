#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class RayTracingRenderer : public Renderer
{
public:
    RayTracingRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test