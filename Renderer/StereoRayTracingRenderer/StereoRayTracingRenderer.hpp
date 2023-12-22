#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class StereoRayTracingRenderer : public Renderer
{
public:
    StereoRayTracingRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test