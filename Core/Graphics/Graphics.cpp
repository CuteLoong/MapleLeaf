#include "Graphics.hpp"
#include "CommandBuffer.hpp"
#include "Devices.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "RenderStage.hpp"
#include "Surface.hpp"
#include "Window.hpp"
#include "glslang/Public/ShaderLang.h"
#include <memory>

#include "config.h"

namespace MapleLeaf {
Graphics::Graphics()
    : instance(std::make_unique<Instance>())
    , physicalDevice(std::make_unique<PhysicalDevice>(*instance))
    , logicalDevice(std::make_unique<LogicalDevice>(*instance, *physicalDevice))
{
    Window* window = Devices::Get()->CreateWindow();
    surface        = std::make_unique<Surface>(*instance, *physicalDevice, *logicalDevice, *window);

    CreatePipelineCache();
    if (!glslang::InitializeProcess()) throw std::runtime_error("Failed to initialize glslang process");
}
Graphics::~Graphics()
{
    auto graphicsQueue = logicalDevice->GetGraphicsQueue();
    auto computeQueue  = logicalDevice->GetComputeQueue();

    CheckVk(vkQueueWaitIdle(graphicsQueue));
    CheckVk(vkQueueWaitIdle(computeQueue));

    renderer  = nullptr;
    swapchain = nullptr;
    surface   = nullptr;

    glslang::FinalizeProcess();
    vkDestroyPipelineCache(*logicalDevice, pipelineCache, nullptr);

    commandPools.clear();
}

const std::shared_ptr<CommandPool>& Graphics::GetCommandPool(const std::thread::id& threadId)
{
    if (auto it = commandPools.find(threadId); it != commandPools.end()) return it->second;
    return commandPools.emplace(threadId, std::make_shared<CommandPool>(threadId)).first->second;
}

void Graphics::CreatePipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    CheckVk(vkCreatePipelineCache(*logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

const RenderStage* Graphics::GetRenderStage(uint32_t index) const
{
    if (renderer) return renderer->GetRenderStage(index);
    return nullptr;
}

const Descriptor* Graphics::GetAttachment(const std::string& name) const
{
    if (auto it = attachments.find(name); it != attachments.end()) return it->second;
    return nullptr;
}

void Graphics::Update()
{
    if (!renderer || Devices::Get()->GetWindow()->IsIconified()) return;

    if (!renderer->started) {
        ResetRenderStages();
        renderer->Start();
        renderer->started = true;
    }

    renderer->Update();

    if (swapchain) {
        auto acquireResult =
            swapchain->AcquireNextImage(surface->presentCompletes[surface->currentFrameIndex], surface->flightFences[surface->currentFrameIndex]);

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }

        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
            Log::Error("Failed to acquire swap chain image!\n");
            return;
        }

        Pipeline::Stage stage;

        for (auto& renderStage : renderer->renderStages) {
            renderStage->Update();

            if (!StartRecordCommandBuffer(*renderStage)) {
                return;
            }

            auto& commandBuffer = surface->commandBuffers[swapchain->GetActiveImageIndex()];

            // now postRender only support compute pass, TODO add support of default pipeline render
            for (const auto& subpass : renderStage->GetSubpasses()) {
                stage.second = subpass.GetBinding();

                // Compute Pass.
                renderer->subrenderHolder.PreRenderStage(stage, *commandBuffer);
            }

            StartRenderpass(*renderStage);
            for (const auto& subpass : renderStage->GetSubpasses()) {
                stage.second = subpass.GetBinding();

                // Renders subpass subrender pipelines.
                renderer->subrenderHolder.RenderStage(stage, *commandBuffer);

                if (subpass.GetBinding() != renderStage->GetSubpasses().back().GetBinding())
                    vkCmdNextSubpass(*commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
            }
            EndRenderpass(*renderStage);

            // now postRender only support compute pass, TODO add support of default pipeline render
            for (const auto& subpass : renderStage->GetSubpasses()) {
                stage.second = subpass.GetBinding();

                // Compute Pass.
                renderer->subrenderHolder.PostRenderStage(stage, *commandBuffer);
            }
            EndRecordCommandBuffer(*renderStage);
            stage.first++;
        }
    }

    // Purges unused command pools.
    if (elapsedPurge.GetElapsed() != 0) {
        for (auto it = commandPools.begin(); it != commandPools.end();) {
            if ((*it).second.use_count() <= 1) {
                it = commandPools.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void Graphics::ResetRenderStages()
{
    RecreateSwapchain();

    if (swapchain) {
        if (surface->GetFilghtFences().size() != swapchain->GetImageCount()) RecreateCommandBuffers();

        for (const auto& renderStage : renderer->renderStages) {
            renderStage->Rebuild(*swapchain);
        }
    }
    RecreateAttachmentsMap();
}

void Graphics::RecreateSwapchain()
{
    vkDeviceWaitIdle(*logicalDevice);
    VkExtent2D displayExtent = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};
#ifdef MAPLELEAF_GRAPHIC_DEBUG
    if (swapchain) {
        Log::Out("Recreating swapchain old (",
                 swapchain->GetExtent().width,
                 ", ",
                 swapchain->GetExtent().height,
                 ") new (",
                 displayExtent.width,
                 ", ",
                 displayExtent.height,
                 ")\n");
    }
#endif
    if (surface) {
        swapchain = std::make_unique<Swapchain>(*physicalDevice, *surface, *logicalDevice, displayExtent, swapchain.get());
        RecreateCommandBuffers();
    }
}

void Graphics::RecreateCommandBuffers()
{
    for (std::size_t i = 0; i < surface->flightFences.size(); i++) {
        vkDestroyFence(*logicalDevice, surface->flightFences[i], nullptr);
        vkDestroySemaphore(*logicalDevice, surface->presentCompletes[i], nullptr);
        vkDestroySemaphore(*logicalDevice, surface->renderCompletes[i], nullptr);
    }

    surface->SetPresentSemaphoreSize(swapchain->GetImageCount());
    surface->SetRenderSemaphoreSize(swapchain->GetImageCount());
    surface->SetFilghtFenceSize(swapchain->GetImageCount());
    surface->SetCommandBufferSize(swapchain->GetImageCount());

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < surface->GetFilghtFences().size(); i++) {
        CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &surface->presentCompletes[i]));
        CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &surface->renderCompletes[i]));
        CheckVk(vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &surface->flightFences[i]));

        surface->commandBuffers[i] = std::make_unique<CommandBuffer>(false);
    }
}

void Graphics::RecreatePass(RenderStage& renderStage)
{
    auto graphicsQueue = logicalDevice->GetGraphicsQueue();

    VkExtent2D displayExtent = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};

    CheckVk(vkQueueWaitIdle(graphicsQueue));

    if (swapchain) {
        if (renderStage.HasSwapchain() && (surface->GetFramebufferResized() || !swapchain->IsSameExtent(displayExtent))) {
            RecreateSwapchain();
        }
        renderStage.Rebuild(*swapchain);
    }
    RecreateAttachmentsMap();
}

void Graphics::RecreateAttachmentsMap()
{
    attachments.clear();

    for (const auto& renderStage : renderer->renderStages) {
        attachments.insert(renderStage->descriptors.begin(), renderStage->descriptors.end());
    }
}

bool Graphics::StartRecordCommandBuffer(RenderStage& renderStage)
{
    if (renderStage.IsOutOfDate()) {
        RecreatePass(renderStage);
        return false;
    }

    auto& commandBuffer = surface->commandBuffers[swapchain->GetActiveImageIndex()];

    if (!commandBuffer->IsRunning()) commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    return true;
}

void Graphics::StartRenderpass(RenderStage& renderStage)
{
    auto& commandBuffer = surface->commandBuffers[swapchain->GetActiveImageIndex()];

    VkRect2D renderArea = {};
    renderArea.offset   = {renderStage.GetRenderArea().GetOffset().x, renderStage.GetRenderArea().GetOffset().y};
    renderArea.extent   = {renderStage.GetRenderArea().GetExtent().x, renderStage.GetRenderArea().GetExtent().y};

    if (renderStage.GetRenderStageType() == RenderStage::Type::MONO) {
        VkViewport viewport = {};
        viewport.x          = 0.0f;
        viewport.y          = static_cast<float>(renderArea.extent.height);
        viewport.width      = static_cast<float>(renderArea.extent.width);
        viewport.height     = -static_cast<float>(renderArea.extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;
        vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset   = renderArea.offset;
        scissor.extent   = renderArea.extent;

        vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
    }
    else if (renderStage.GetRenderStageType() == RenderStage::Type::STEREO) {
        std::array<VkRect2D, 2> renderArea = {};
        renderArea[0].offset               = {renderStage.GetRenderArea().GetOffset().x, renderStage.GetRenderArea().GetOffset().y};
        renderArea[0].extent               = {renderStage.GetRenderArea().GetExtent().x / 2, renderStage.GetRenderArea().GetExtent().y};
        renderArea[1].offset = {static_cast<int32_t>(renderStage.GetRenderArea().GetExtent().x / 2 + renderStage.GetRenderArea().GetOffset().x),
                                renderStage.GetRenderArea().GetOffset().y};
        renderArea[1].extent = {renderStage.GetRenderArea().GetExtent().x / 2, renderStage.GetRenderArea().GetExtent().y};

        std::array<VkViewport, 2> viewport{};
        viewport[0].x        = 0.0f;
        viewport[0].y        = static_cast<float>(renderArea[0].extent.height);
        viewport[0].width    = static_cast<float>(renderArea[0].extent.width);
        viewport[0].height   = -static_cast<float>(renderArea[0].extent.height);
        viewport[0].minDepth = 0.0f;
        viewport[0].maxDepth = 1.0f;

        viewport[1].x        = static_cast<float>(renderArea[0].extent.width);
        viewport[1].y        = static_cast<float>(renderArea[1].extent.height);
        viewport[1].width    = static_cast<float>(renderArea[1].extent.width);
        viewport[1].height   = -static_cast<float>(renderArea[1].extent.height);
        viewport[1].minDepth = 0.0f;
        viewport[1].maxDepth = 1.0f;
        vkCmdSetViewport(*commandBuffer, 0, 2, viewport.data());

        std::array<VkRect2D, 2> scissor{};
        scissor[0].offset = renderArea[0].offset;
        scissor[0].extent = renderArea[0].extent;

        scissor[1].offset = renderArea[1].offset;
        scissor[1].extent = renderArea[1].extent;
        vkCmdSetScissor(*commandBuffer, 0, 2, scissor.data());
    }

    auto clearValues = renderStage.GetClearValues();

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *renderStage.GetRenderpass();
    renderPassBeginInfo.framebuffer           = renderStage.GetActiveFramebuffer(swapchain->GetActiveImageIndex());
    renderPassBeginInfo.renderArea            = renderArea;
    renderPassBeginInfo.clearValueCount       = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues          = clearValues.data();
    vkCmdBeginRenderPass(*commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Graphics::EndRenderpass(RenderStage& renderStage)
{
    auto& commandBuffer = surface->commandBuffers[swapchain->GetActiveImageIndex()];
    vkCmdEndRenderPass(*commandBuffer);
}

void Graphics::EndRecordCommandBuffer(RenderStage& renderStage)
{
    auto  presentQueue  = logicalDevice->GetPresentQueue();
    auto& commandBuffer = surface->commandBuffers[swapchain->GetActiveImageIndex()];

    if (!renderStage.HasSwapchain()) return;

    commandBuffer->End();
    commandBuffer->Submit(surface->presentCompletes[surface->currentFrameIndex],
                          surface->renderCompletes[surface->currentFrameIndex],
                          surface->flightFences[surface->currentFrameIndex]);

    auto presentResult = swapchain->QueuePresent(presentQueue, surface->renderCompletes[surface->currentFrameIndex]);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        surface->framebufferResized = true;
    }
    else if (presentResult != VK_SUCCESS) {
        CheckVk(presentResult);
        Log::Error("Failed to present swap chain image!\n");
    }

    surface->currentFrameIndex = (surface->currentFrameIndex + 1) % swapchain->GetImageCount();
}

std::string Graphics::StringifyResultVk(VkResult result)
{
    switch (result) {
    case VK_SUCCESS: return "Success";
    case VK_NOT_READY: return "A fence or query has not yet completed";
    case VK_TIMEOUT: return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET: return "An event is signaled";
    case VK_EVENT_RESET: return "An event is unsignaled";
    case VK_INCOMPLETE: return "A return array was too small for the result";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "A host memory allocation has failed";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "A device memory allocation has failed";
    case VK_ERROR_INITIALIZATION_FAILED: return "Initialization of an object could not be completed for implementation-specific reasons";
    case VK_ERROR_DEVICE_LOST: return "The logical or physical device has been lost";
    case VK_ERROR_MEMORY_MAP_FAILED: return "Mapping of a memory object has failed";
    case VK_ERROR_LAYER_NOT_PRESENT: return "A requested layer is not present or could not be loaded";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "A requested extension is not supported";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "A requested feature is not supported";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
    case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects of the type have already been created";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "A requested format is not supported on this device";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available";
        // case VK_ERROR_OUT_OF_POOL_MEMORY:
        //	return "A allocation failed due to having no more space in the descriptor pool";
    case VK_SUBOPTIMAL_KHR: return "A swapchain no longer matches the surface properties exactly, but can still be used";
    case VK_ERROR_OUT_OF_DATE_KHR: return "A surface has changed in such a way that it is no longer compatible with the swapchain";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "The display used by a swapchain does not use the same presentable image layout";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "A validation layer found an error";
    default: return "Unknown Vulkan error";
    }
}
void Graphics::CheckVk(VkResult result)
{
    if (result >= 0) return;

    auto failure = StringifyResultVk(result);

    throw std::runtime_error("Vulkan error: " + failure);
}
}   // namespace MapleLeaf