#include "RenderStage.hpp"
#include "Devices.hpp"
#include "Graphics.hpp"
#include "Log.hpp"

#include "config.h"

namespace MapleLeaf {
RenderStage::RenderStage(std::vector<Attachment> images, std::vector<SubpassType> subpasses, const Viewport& viewport)
    : attachments(std::move(images))
    , subpasses(std::move(subpasses))
    , viewport(viewport)
    , subpassOutputAttachmentCount(this->subpasses.size())
    , subpassInputAttachmentCount(this->subpasses.size())
    , subpassMultisampled(this->subpasses.size())
{
    for (const auto& image : attachments) {
        VkClearValue clearValue = {};

        switch (image.GetType()) {
        case Attachment::Type::Image:
            clearValue.color = {{image.GetClearColour().r, image.GetClearColour().g, image.GetClearColour().b, image.GetClearColour().a}};
            for (const auto& subpass : this->subpasses) {
                if (auto subpassBindings = subpass.GetInputAttachmentBindings();
                    std::find(subpassBindings.begin(), subpassBindings.end(), image.GetBinding()) != subpassBindings.end()) {
                    subpassInputAttachmentCount[subpass.GetBinding()]++;
                }
                if (auto subpassBindings = subpass.GetOutputAttachmentBindings();
                    std::find(subpassBindings.begin(), subpassBindings.end(), image.GetBinding()) != subpassBindings.end()) {
                    subpassOutputAttachmentCount[subpass.GetBinding()]++;

                    if (image.IsMultisampled()) subpassMultisampled[subpass.GetBinding()] = true;
                }
            }

            break;
        case Attachment::Type::Depth:
            clearValue.depthStencil = {1.0f, 0};
            depthAttachment         = image;
            break;
        case Attachment::Type::Swapchain:
            clearValue.color    = {{image.GetClearColour().r, image.GetClearColour().g, image.GetClearColour().b, image.GetClearColour().a}};
            swapchainAttachment = image;
            break;
        }

        clearValues.emplace_back(clearValue);
    }
}

void RenderStage::Update()
{
    auto lastRenderArea = renderArea;

    renderArea.SetOffset(viewport.GetOffset());

    if (viewport.GetSize())
        renderArea.SetExtent({viewport.GetScale().x * viewport.GetSize()->x, viewport.GetScale().y * viewport.GetSize()->y});
    else
        renderArea.SetExtent(
            {viewport.GetScale().x * Devices::Get()->GetWindow()->GetSize().x, viewport.GetScale().y * Devices::Get()->GetWindow()->GetSize().y});

    renderArea.SetAspectRatio(static_cast<float>(renderArea.GetExtent().x) / static_cast<float>(renderArea.GetExtent().y));
    renderArea.SetExtent({renderArea.GetExtent().x + renderArea.GetOffset().x, renderArea.GetExtent().y + renderArea.GetOffset().y});

    outOfDate = renderArea != lastRenderArea;
}

void RenderStage::Rebuild(const Swapchain& swapchain)
{
#ifdef MAPLELEAF_RENDERSTAGE_DEBUG
    auto debugStart = Time::Now();
#endif
    Update();

    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();
    auto logicalDevice  = Graphics::Get()->GetLogicalDevice();
    auto surface        = Graphics::Get()->GetSurface();

    auto msaaSamples = physicalDevice->GetMsaaSamples();

    if (depthAttachment)
        depthStencil = std::make_unique<ImageDepth>(renderArea.GetExtent(), depthAttachment->IsMultisampled() ? msaaSamples : VK_SAMPLE_COUNT_1_BIT);

    if (!renderpass)
        renderpass = std::make_unique<Renderpass>(
            *logicalDevice, *this, depthStencil ? depthStencil->GetFormat() : VK_FORMAT_UNDEFINED, surface->GetFormat().format, msaaSamples);

    framebuffers = std::make_unique<Framebuffers>(*logicalDevice, swapchain, *this, *renderpass, *depthStencil, renderArea.GetExtent(), msaaSamples);
    outOfDate    = false;

    descriptors.clear();
    auto where = descriptors.end();

    for (const auto& image : attachments) {
        if (image.GetType() == Attachment::Type::Depth)
            where = descriptors.insert(where, {image.GetName(), depthStencil.get()});
        else
            where = descriptors.insert(where, {image.GetName(), framebuffers->GetAttachment(image.GetBinding())});
    }

#ifdef MAPLELEAF_RENDERSTAGE_DEBUG
    Log::Out("Render Stage created in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

std::optional<Attachment> RenderStage::GetAttachment(const std::string& name) const
{
    auto it = std::find_if(attachments.begin(), attachments.end(), [name](const Attachment& a) { return a.GetName() == name; });

    if (it != attachments.end()) return *it;
    return std::nullopt;
}
std::optional<Attachment> RenderStage::GetAttachment(uint32_t binding) const
{
    auto it = std::find_if(attachments.begin(), attachments.end(), [binding](const Attachment& a) { return a.GetBinding() == binding; });

    if (it != attachments.end()) return *it;
    return std::nullopt;
}

const VkFramebuffer& RenderStage::GetActiveFramebuffer(uint32_t activeSwapchainImage) const
{
    if (activeSwapchainImage > framebuffers->GetFramebuffers().size()) return framebuffers->GetFramebuffers().at(0);

    return framebuffers->GetFramebuffers().at(activeSwapchainImage);
}
}   // namespace MapleLeaf