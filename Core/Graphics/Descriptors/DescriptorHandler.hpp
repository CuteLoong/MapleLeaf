#pragma once

#include "Descriptor.hpp"
#include "DescriptorSet.hpp"
#include "Shader.hpp"

namespace MapleLeaf {
class DescriptorsHandler
{
public:
private:
    class DescriptorValue
    {
    public:
        const Descriptor*         descriptor;
        WriteDescriptorSet        writeDescriptor;
        std::optional<OffsetSize> offsetSize;
        uint32_t                  location;
    };

    const Shader*                  shader          = nullptr;
    bool                           pushDescriptors = false;
    std::unique_ptr<DescriptorSet> descriptorSet;

    std::map<std::string, DescriptorValue> descriptors;
    std::vector<VkWriteDescriptorSet>      writeDescriptorSets;
    bool                                   changed = false;
};
}   // namespace MapleLeaf