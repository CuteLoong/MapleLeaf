#include "DescriptorHandler.hpp"

#include "Graphics.hpp"
#include "Pipeline.hpp"

namespace MapleLeaf {
DescriptorsHandler::DescriptorsHandler(const Pipeline& pipeline)
    : shader(pipeline.GetShader())
    , pushDescriptors(pipeline.IsPushDescriptors())
    , descriptorSets(std::make_unique<DescriptorSets>(pipeline))
    , changed(true)
{}

void DescriptorsHandler::Push(const std::string& descriptorName, UniformHandler& uniformHandler, const std::optional<uint32_t> descriptorArrayIndex,
                              const std::optional<OffsetSize>& offsetSize)
{
    if (shader) {
        uniformHandler.Update(shader->GetUniformBlock(descriptorName));
        Push(descriptorName, uniformHandler.GetUniformBuffer(), descriptorArrayIndex, offsetSize);
    }
}

void DescriptorsHandler::Push(const std::string& descriptorName, StorageHandler& storageHandler, const std::optional<uint32_t> descriptorArrayIndex,
                              const std::optional<OffsetSize>& offsetSize)
{
    if (shader) {
        storageHandler.Update(shader->GetUniformBlock(descriptorName));
        Push(descriptorName, storageHandler.GetStorageBuffer(), descriptorArrayIndex, offsetSize);
    }
}

void DescriptorsHandler::Push(const std::string& descriptorName, IndirectHandler& indirectHandler, const std::optional<uint32_t> descriptorArrayIndex,
                              const std::optional<OffsetSize>& offsetSize)
{
    if (shader) {
        indirectHandler.Update(shader->GetUniformBlock(descriptorName));
        Push(descriptorName, indirectHandler.GetIndirectBuffer(), descriptorArrayIndex, offsetSize);
    }
}

void DescriptorsHandler::Push(const std::string& descriptorName, PushHandler& pushHandler, const std::optional<uint32_t> descriptorArrayIndex,
                              const std::optional<OffsetSize>& offsetSize)
{
    if (shader) {
        pushHandler.Update(shader->GetUniformBlock(descriptorName));
    }
}

void DescriptorsHandler::Push(const std::string& descriptorName, const Image* MipmapImage, const std::optional<uint32_t> mipLevel,
                              const std::optional<uint32_t> descriptorArrayIndex, const std::optional<OffsetSize>& offsetSize)
{
    if (shader) {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler               = MipmapImage->GetSampler();
        imageInfo.imageView             = MipmapImage->GetMipView(mipLevel.value());
        imageInfo.imageLayout           = MipmapImage->GetLayout();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet               = VK_NULL_HANDLE;   // Will be set in the descriptor handler.
        descriptorWrite.dstBinding           = *shader->GetDescriptorLocation(descriptorName).second;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorCount      = 1;
        descriptorWrite.descriptorType = *shader->GetDescriptorType(*shader->GetDescriptorLocation(descriptorName).first, descriptorWrite.dstBinding);

        WriteDescriptorSet writeDescriptorSet(descriptorWrite, imageInfo);
        Push(descriptorName, MipmapImage, std::move(writeDescriptorSet));
    }
}

bool DescriptorsHandler::Update(const Pipeline& pipeline)
{
    if (shader != pipeline.GetShader()) {
        shader          = pipeline.GetShader();
        pushDescriptors = pipeline.IsPushDescriptors();
        descriptors.clear();
        writeDescriptorSets.clear();

        if (!pushDescriptors) {
            descriptorSets = std::make_unique<DescriptorSets>(pipeline);
        }

        changed = false;
        return false;
    }

    if (changed) {
        writeDescriptorSets.clear();
        writeDescriptorSets.reserve(descriptors.size());

        // this descriptor meaning maybe have multiple descriptor, e.g this descriptor is a descriptor array
        for (const auto& [descriptorName, descriptor] : descriptors) {
            uint32_t dstArrayElement = 0;
            for (const auto& descriptorElem : descriptor) {
                auto writeDescriptorSet   = descriptorElem.writeDescriptor.GetWriteDescriptorSet();
                writeDescriptorSet.dstSet = VK_NULL_HANDLE;

                // TODO. only update a certein of writeDescriptorSet
                if (!pushDescriptors) {
                    writeDescriptorSet.dstSet          = descriptorSets->GetDescriptorSet(descriptorElem.set);
                    writeDescriptorSet.dstArrayElement = dstArrayElement;
                }

                writeDescriptorSets.emplace_back(writeDescriptorSet);
                dstArrayElement++;
            }
        }

        if (!pushDescriptors) descriptorSets->Update(writeDescriptorSets);

        changed = false;
    }

    return true;
}

void DescriptorsHandler::BindDescriptor(const CommandBuffer& commandBuffer, const Pipeline& pipeline)
{
    if (pushDescriptors) {
        auto logicalDevice = Graphics::Get()->GetLogicalDevice();
        Instance::FvkCmdPushDescriptorSetKHR(*logicalDevice,
                                             commandBuffer,
                                             pipeline.GetPipelineBindPoint(),
                                             pipeline.GetPipelineLayout(),
                                             0,
                                             static_cast<uint32_t>(writeDescriptorSets.size()),
                                             writeDescriptorSets.data());
    }
    else {
        descriptorSets->BindDescriptor(commandBuffer);
    }
}
}   // namespace MapleLeaf