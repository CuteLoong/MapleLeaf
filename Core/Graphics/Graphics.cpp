#include "Graphics.hpp"
#include "CommandBuffer.hpp"
#include "Devices.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Window.hpp"
#include <memory>


namespace MapleLeaf {
Graphics::Graphics()
    : instance(std::make_unique<Instance>())
    , physicalDevice(std::make_unique<PhysicalDevice>(*instance))
    , logicalDevice(std::make_unique<LogicalDevice>(*instance, *physicalDevice))
{
    Window* window = Devices::Get()->CreateWindow();
    surface        = std::make_unique<Surface>(*instance, *physicalDevice, *logicalDevice, *window);
}
Graphics::~Graphics()
{
    auto graphicsQueue = logicalDevice->GetGraphicsQueue();

    renderer = nullptr;
}

const std::shared_ptr<CommandPool>& Graphics::GetCommandPool()
{
    if (!commandPool) commandPool = std::make_shared<CommandPool>(0);
    return commandPool;
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
}

void Graphics::ResetRenderStages()
{
    RecreateSwapchain();

    if (surface->GetFilghtFences().size() != swapchain->GetImageCount()) RecreateCommandBuffers();

    for (const auto& renderStage : renderer->renderStages) {
        renderStage->Rebuild(*swapchain);
    }
    // reset swapchain and attachment
}

void Graphics::RecreateSwapchain()
{
    vkDeviceWaitIdle(*logicalDevice);
    VkExtent2D displayExtent = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};
#ifdef MAPLELEAF_DEBUG
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
    swapchain = std::make_unique<Swapchain>(*physicalDevice, *surface, *logicalDevice, displayExtent, swapchain.get());
    // update semaphore and commandbuffers
}

void Graphics::RecreateCommandBuffers()
{
    for (std::size_t i = 0; i < surface->GetFilghtFences().size(); i++) {
        vkDestroyFence(*logicalDevice, surface->GetFilghtFences()[i], nullptr);
        vkDestroySemaphore(*logicalDevice, surface->GetPresentSemaphores()[i], nullptr);
        vkDestroySemaphore(*logicalDevice, surface->GetRenderSemaphores()[i], nullptr);
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
        CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &surface->GetPresentSemaphores()[i]));
        CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &surface->GetRenderSemaphores()[i]));
        CheckVk(vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &surface->GetFilghtFences()[i]));

        surface->GetCommandBuffers()[i] = std::make_unique<CommandBuffer>(false);
    }
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