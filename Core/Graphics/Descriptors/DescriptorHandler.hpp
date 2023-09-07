#pragma once

#include "Descriptor.hpp"
#include "DescriptorSet.hpp"
#include "PushHandler.hpp"
#include "Shader.hpp"
#include "StorageHandler.hpp"
#include "UniformHandler.hpp"
#include "Utils.hpp"
#include <memory>


namespace MapleLeaf {
class DescriptorsHandler
{
public:
    DescriptorsHandler() = default;
    explicit DescriptorsHandler(const Pipeline& pipeline);

    /**
     * @brief Push will be used by handler's Update
     *
     * @tparam T
     * @param descriptorName
     * @param descriptor is a pointer to uniformBuffer or storageBuffer
     * @param offsetSize
     */
    template<typename T>
    void Push(const std::string& descriptorName, const T& descriptor, const std::optional<OffsetSize>& offsetSize = std::nullopt)
    {
        if (!shader) return;

        if (auto it = descriptors.find(descriptorName); it != descriptors.end()) {
            if (it->second.descriptor == to_address(descriptor) && it->second.offsetSize == offsetSize) return;

            descriptors.erase(it);
        }

        if (!to_address(descriptor)) return;

        auto setLocation = shader->GetDescriptorLocation(descriptorName);
        auto setIndex    = setLocation.first;
        auto location    = setLocation.second;

        if (!setIndex || !location) {
#ifdef MAPLELEAF_DEBUG
            if (shader->ReportedNotFound(descriptorName, true))
                Log::Error("Could not find descriptor in shader ", shader->GetName(), " of name ", std::quoted(descriptorName), '\n');
#endif
            return;
        }

        auto descriptorType = shader->GetDescriptorType(setIndex.value(), location.value());

        if (!descriptorType) {
#ifdef MAPLELEAF_DEBUG
            if (shader->ReportedNotFound(descriptorName, true))
                Log::Error("Could not find descriptor in shader ",
                           shader->GetName(),
                           " of name ",
                           std::quoted(descriptorName),
                           " at location ",
                           *location,
                           '\n');
#endif
            return;
        }

        auto writeDescriptor = to_address(descriptor)->GetWriteDescriptor(location.value(), descriptorType.value(), offsetSize);
        descriptors.emplace(descriptorName,
                            DescriptorValue(to_address(descriptor), std::move(writeDescriptor), offsetSize, setIndex.value(), location.value()));
        changed = true;
    }

    template<typename T>
    void Push(const std::string& descriptorName, const T& descriptor, WriteDescriptorSet writeDescriptorSet)
    {
        if (!shader) return;

        if (auto it = descriptors.find(descriptorName); it != descriptors.end()) {
            descriptors.erase(it);
        }

        auto setLocation = shader->GetDescriptorLocation(descriptorName);
        auto setIndex    = setLocation.first;
        auto location    = setLocation.second;

        descriptors.emplace(descriptorName,
                            DescriptorValue{to_address(descriptor), std::move(writeDescriptorSet), std::nullopt, setIndex.value(), location.value()});
        changed = true;
    }

    void Push(const std::string& descriptorName, UniformHandler& uniformHandler, const std::optional<OffsetSize>& offsetSize = std::nullopt);
    void Push(const std::string& descriptorName, StorageHandler& storageHandler, const std::optional<OffsetSize>& offsetSize = std::nullopt);
    void Push(const std::string& descriptorName, PushHandler& pushHandler, const std::optional<OffsetSize>& offsetSize = std::nullopt);

    bool Update(const Pipeline& pipeline);

    void                 BindDescriptor(const CommandBuffer& commandBuffer, const Pipeline& pipeline);
    const DescriptorSet* GetDescriptorSet() const { return descriptorSet.get(); }

private:
    class DescriptorValue
    {
    public:
        const Descriptor*         descriptor;
        WriteDescriptorSet        writeDescriptor;
        std::optional<OffsetSize> offsetSize;
        uint32_t                  location;
        uint32_t                  set;

        DescriptorValue(const Descriptor* descriptor, WriteDescriptorSet&& writeDescriptor, std::optional<OffsetSize> offsetSize, uint32_t set,
                        uint32_t location)
            : descriptor(descriptor)
            , writeDescriptor(std::move(writeDescriptor))
            , offsetSize(offsetSize)
            , set(set)
            , location(location)
        {}
    };

    const Shader*                  shader          = nullptr;
    bool                           pushDescriptors = false;
    std::unique_ptr<DescriptorSet> descriptorSet;

    std::map<std::string, DescriptorValue> descriptors;
    std::vector<VkWriteDescriptorSet>      writeDescriptorSets;
    bool                                   changed = false;
};
}   // namespace MapleLeaf