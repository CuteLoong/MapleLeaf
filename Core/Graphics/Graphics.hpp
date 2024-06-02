#pragma once

#include "CommandBuffer.hpp"
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

    const std::shared_ptr<CommandPool>& GetCommandPool(const std::thread::id& threadId = std::this_thread::get_id());

    Renderer* GetRenderer() const { return renderer.get(); }
    void      SetRenderer(std::unique_ptr<Renderer>&& renderer) { this->renderer = std::move(renderer); }

    const glm::uvec2& GetNonRTAttachmentSize() const { return renderer->GetNonRTAttachmentsHandler()->GetFrameAttachmentSize(); }
    const Descriptor* GetNonRTAttachment(const std::string& name) const { return renderer->GetNonRTAttachmentsHandler()->GetDescriptor(name); }

    const RenderStage* GetRenderStage(uint32_t index) const;
    const Descriptor*  GetAttachment(const std::string& name) const;

    static std::string StringifyResultVk(VkResult result);
    static void        CheckVk(VkResult result);

    void CaptureScreenshot(const std::filesystem::path& filename);

private:
    std::unique_ptr<Renderer>                renderer;
    std::map<std::string, const Descriptor*> attachments;
    std::unique_ptr<Instance>                instance;

    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice>  logicalDevice;
    std::unique_ptr<Swapchain>      swapchain;
    std::unique_ptr<Surface>        surface;

    std::map<std::thread::id, std::shared_ptr<CommandPool>> commandPools;
    // Timer used to remove unused command pools.
    ElapsedTime elapsedPurge;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    void CreatePipelineCache();
    void ResetRenderStages();
    void RecreateSwapchain();
    void RecreateCommandBuffers();
    void RecreatePass(RenderStage& renderStage);
    void RecreateAttachmentsMap();
    bool StartRecordCommandBuffer(RenderStage& renderStage);
    void StartRenderpass(RenderStage& renderStage);
    void EndRenderpass(RenderStage& renderStage);
    void EndRecordCommandBuffer(RenderStage& renderStage);
};
}   // namespace MapleLeaf