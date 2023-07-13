#pragma once

#include "Engine.hpp"
#include "Module.hpp"
#include "Window.hpp"

namespace MapleLeaf {
class Devices : public Module::Registrar<Devices>
{
    inline static const bool Registered = Register(Stage::Pre);

public:
    Devices();
    ~Devices();

    void Update() override;

    Window*       CreateWindow();
    const Window* GetWindow() const;
    Window*       GetWindow();

    std::pair<const char**, uint32_t> GetInstanceExtensions() const;

private:
    std::unique_ptr<Window> window;
};
}   // namespace MapleLeaf