#pragma once

#include "Renderer.hpp"

using namespace MapleLeaf;

namespace Test {
class RawSSRRenderer : public Renderer
{
public:
    RawSSRRenderer();

    void Start() override;
    void Update() override;
};
}   // namespace Test