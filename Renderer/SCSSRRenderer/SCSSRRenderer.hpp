#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class SCSSRRenderer : public Renderer
{
public:
    SCSSRRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test