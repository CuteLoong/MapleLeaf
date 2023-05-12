#pragma once

#include "Devices.hpp"
#include "Engine.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Renderer.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include <memory>


namespace MapleLeaf {
class Graphics : public Module::Registrar<Graphics>
{
    inline static const bool Registered = Register(Stage::Render, Requires<Devices>());

public:
    Graphics();
    ~Graphics();

    void Update() override;

    const PhysicalDevice* GetPhysicalDevice() const { return physicalDevice.get(); }
    const LogicalDevice*  GetLogicalDevice() const { return logicalDevice.get(); }
    const Surface*        GetSurface() const { return surface.get(); }

    void SetRenderer(std::unique_ptr<Renderer>&& renderer) { this->renderer = std::move(renderer); }

    static std::string StringifyResultVk(VkResult result);
    static void        CheckVk(VkResult result);

private:
    std::unique_ptr<Renderer> renderer;

    std::unique_ptr<Instance> instance;

    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice>  logicalDevice;
    std::unique_ptr<Swapchain>      swapchain;
    std::unique_ptr<Surface>        surface;

    void ResetRenderStages();
    void RecreateSwapchain();
};
}   // namespace MapleLeaf