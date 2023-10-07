#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class MainRenderer : public Renderer
{
public:
    MainRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test