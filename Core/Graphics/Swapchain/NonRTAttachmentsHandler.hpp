#pragma once

#include "Color.hpp"
#include "Image2d.hpp"
#include "ImageCube.hpp"
#include "ImageHierarchyZ.hpp"
#include "NonCopyable.hpp"
#include <unordered_map>

namespace MapleLeaf {
// it's size is same with swapchain, but not a render target will be used globally.
class NonRTAttachment
{
    friend class NonRTAttachmentsHandler;

public:
    enum class Type
    {
        Image2d,
        ImageCube,
        ImageHierarchyZ
    };

    NonRTAttachment(std::string name, Type type, bool multisampled = false, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                    VkFilter filter = VK_FILTER_LINEAR, const Color& clearColour = Color::Black, std::optional<glm::uvec2> fixedSize = std::nullopt)
        : name(std::move(name))
        , type(type)
        , multisampled(multisampled)
        , format(format)
        , filter(filter)
        , clearColour(clearColour)
        , fixedSize(fixedSize)
    {}

    const std::string& GetName() const { return name; }
    Type               GetType() const { return type; }
    bool               IsMultisampled() const { return multisampled; }
    VkFormat           GetFormat() const { return format; }
    VkFilter           GetFilter() const { return filter; }
    const Color&       GetClearColour() const { return clearColour; }

private:
    std::string               name;
    Type                      type;
    bool                      multisampled;
    VkFormat                  format;
    VkFilter                  filter;
    Color                     clearColour;
    std::optional<glm::uvec2> fixedSize;   // swapchain size or fixed size
};

class NonRTAttachmentsHandler : NonCopyable
{
public:
    explicit NonRTAttachmentsHandler(const std::vector<NonRTAttachment>& NonRTAttachmentTypes);
    ~NonRTAttachmentsHandler() = default;

    void Update();

    const Descriptor* GetDescriptor(const std::string& name) const { return NonRTImages.at(name).get(); }
    const glm::uvec2& GetFrameAttachmentSize() const { return frameAttachmentSize; }

private:
    std::unordered_map<std::string, std::unique_ptr<Image>> NonRTImages;
    std::vector<NonRTAttachment>                            nonRTAttachments;

    glm::uvec2 frameAttachmentSize = {0, 0};

    void RecreateAttachments();
};
}   // namespace MapleLeaf