#pragma once

#include "Descriptor.hpp"
#include "DescriptorSet.hpp"
#include "Shader.hpp"
#include <iomanip>
#include <memory>
#include <optional>


namespace MapleLeaf {
class DescriptorsHandler
{
public:
    DescriptorsHandler() = default;
    explicit DescriptorsHandler(const Pipeline& pipeline);

    template<typename T>
    void Push(const std::string& descriptorName, const T& descriptor, const std::optional<OffsetSize>& offsetSize = std::nullopt)
    {
        if (!shader) return;

        auto it = descriptors.find(descriptorName);

        if (it != descriptors.end()) {
            if (it->second.descriptor == std::addressof(descriptor) && it->second.offsetSize == descriptor->offsetSize) return;

            descriptors.erase(it);
        }

        if (!std::addressof(descriptor)) return;

        auto location = shader->GetDescriptorLocation(descriptorName);
    }

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