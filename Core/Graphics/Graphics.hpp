#pragma once

#include "CommandPool.hpp"
#include "Devices.hpp"
#include "Engine.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "RenderStage.hpp"
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

    const PhysicalDevice*  GetPhysicalDevice() const { return physicalDevice.get(); }
    const LogicalDevice*   GetLogicalDevice() const { return logicalDevice.get(); }
    const Surface*         GetSurface() const { return surface.get(); }
    const VkPipelineCache& GetPipelineCache() const { return pipelineCache; }

    const std::shared_ptr<CommandPool>& GetCommandPool();

    Renderer* GetRenderer() const { return renderer.get(); }
    void      SetRenderer(std::unique_ptr<Renderer>&& renderer) { this->renderer = std::move(renderer); }

    const RenderStage* GetRenderStage(uint32_t index) const;

    static std::string StringifyResultVk(VkResult result);
    static void        CheckVk(VkResult result);

private:
    std::unique_ptr<Renderer> renderer;

    std::unique_ptr<Instance> instance;

    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice>  logicalDevice;
    std::unique_ptr<Swapchain>      swapchain;
    std::unique_ptr<Surface>        surface;

    std::shared_ptr<CommandPool> commandPool;
    VkPipelineCache              pipelineCache = VK_NULL_HANDLE;

    void CreatePipelineCache();
    void ResetRenderStages();
    void RecreateSwapchain();
    void RecreateCommandBuffers();
    void RecreatePass(RenderStage& renderStage);
    bool StartRenderpass(RenderStage& renderStage);
    void EndRenderpass(RenderStage& renderStage);
};
}   // namespace MapleLeaf