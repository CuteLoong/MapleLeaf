#pragma once

#include "App.hpp"

using namespace MapleLeaf;

namespace Test {
class MainApp : public App
{
public:
    MainApp();
    ~MainApp();

    void Start() override;
    void Update() override;
};
}   // namespace Test