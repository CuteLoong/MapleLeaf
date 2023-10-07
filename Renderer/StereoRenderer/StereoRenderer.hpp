#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class StereoRenderer : public Renderer
{
public:
    StereoRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test