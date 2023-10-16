#pragma once

#include "Color.hpp"
#include "Image2d.hpp"
#include "ImageCube.hpp"
#include "ImageHierarchyZ.hpp"
#include "NonCopyable.hpp"
#include <unordered_map>

namespace MapleLeaf {
// it's size is same with swapchain, but not a render target will be used globally.
class FrameAttachment
{
    friend class GlobalAttachmentsHandler;

public:
    enum class Type
    {
        Image2d,
        ImageCube,
        ImageHierarchyZ
    };

    FrameAttachment(std::string name, Type type, bool multisampled = false, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
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

class GlobalAttachmentsHandler : NonCopyable
{
public:
    explicit GlobalAttachmentsHandler(const std::vector<FrameAttachment>& frameAttachmentTypes);
    ~GlobalAttachmentsHandler() = default;

    void Update();

private:
    std::unordered_map<std::string, std::unique_ptr<Image>> frameAttachments;
    std::vector<FrameAttachment>                            frameAttachmentTypes;

    glm::uvec2 frameAttachmentSize = {0, 0};

    void RecreateAttachments();
};
}   // namespace MapleLeaf