#pragma once

#include "volk.h"
#include <vector>

namespace glslang {
class TProgram;
class TType;
}   // namespace glslang

namespace MapleLeaf {
class Shader
{
public:
    class VertexInput
    {
    public:
        VertexInput(std::vector<VkVertexInputBindingDescription>   bindingDescriptions   = {},
                    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {})
            : bindingDescriptions(std::move(bindingDescriptions))
            , attributeDescriptions(std::move(attributeDescriptions))
        {}

        const std::vector<VkVertexInputBindingDescription>&   GetBindingDescriptions() const { return bindingDescriptions; }
        const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

    private:
        uint32_t                                       binding = 0;
        std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };
};
}   // namespace MapleLeaf