#pragma once

#include "Devices.hpp"
#include "Engine.hpp"
#include "Instance.hpp"


namespace MapleLeaf {
class Graphics : public Module::Registrar<Graphics>
{
    inline static const bool Registered = Register(Stage::Render, Requires<Devices>());

public:
    Graphics();
    ~Graphics();

    void Update() override;

    static std::string StringifyResultVk(VkResult result);
    static void        CheckVk(VkResult result);

private:
    std::unique_ptr<Instance> instance;
};
}   // namespace MapleLeaf