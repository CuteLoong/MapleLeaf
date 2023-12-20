#include "PipelineRayTracing.hpp"

#include "Files.hpp"
#include "Graphics.hpp"
#include "Maths.hpp"

namespace MapleLeaf {
PipelineRayTracing::PipelineRayTracing(std::vector<std::filesystem::path> shaderStages, std::vector<Shader::Define> defines, bool pushDescriptors)
    : shaderStages(std::move(shaderStages))
    , defines(std::move(defines))
    , pushDescriptors(pushDescriptors)
    , shader(std::make_unique<Shader>())
    , pipelineBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
{
#ifdef MAPLELEAF_PIPELINE_DEBUG
    auto debugStart = Time::Now();
#endif
    CreateShaderProgram();
    CreateDescriptorLayout();
    CreateDescriptorPool();
    CreatePipelineLayout();
    CreatePipelineRayTracing();
    CreateShaderBindingTable();

#ifdef MAPLELEAF_PIPELINE_DEBUG
    Log::Out("Pipeline RayTracing ", this->shaderStages.back(), " loaded in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

PipelineRayTracing::~PipelineRayTracing()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    for (const auto& shaderModule : modules) vkDestroyShaderModule(*logicalDevice, shaderModule, nullptr);

    vkDestroyDescriptorPool(*logicalDevice, descriptorPool, nullptr);
    vkDestroyPipeline(*logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(*logicalDevice, pipelineLayout, nullptr);
    for (auto& [setIndex, descriptorSetLayout] : descriptorSetNormalLayouts)
        vkDestroyDescriptorSetLayout(*logicalDevice, descriptorSetLayout, nullptr);
    for (auto& [setIndex, descriptorSetLayout] : descriptorSetBindlessLayouts)
        vkDestroyDescriptorSetLayout(*logicalDevice, descriptorSetLayout, nullptr);
    SBTBuffer.reset();
}

void PipelineRayTracing::CmdRender(const CommandBuffer& commandBuffer, const glm::uvec2& extent) const
{
    vkCmdTraceRaysKHR(commandBuffer, &rgenRegion, &missRegion, &hitRegion, &callRegion, extent.x, extent.y, 1);
}

void PipelineRayTracing::CreateShaderProgram()
{
    std::stringstream defineBlock;
    for (const auto& [defineName, defineValue] : defines) defineBlock << "#define " << defineName << " " << defineValue << '\n';

    for (const auto& shaderStage : shaderStages) {
        auto fileLoaded = Files::Get()->Read(shaderStage);

        if (!fileLoaded) throw std::runtime_error("Could not create pipeline, missing shader stage");

        auto stageFlag = Shader::GetShaderStage(shaderStage);
        if (stageFlag == VK_SHADER_STAGE_MISS_BIT_KHR)
            missShaderCount++;
        else if (stageFlag == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
            hitShaderCount++;
        auto shaderModule = shader->CreateShaderModule(shaderStage, *fileLoaded, defineBlock.str(), stageFlag);

        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
        pipelineShaderStageCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.stage                           = stageFlag;
        pipelineShaderStageCreateInfo.module                          = shaderModule;
        pipelineShaderStageCreateInfo.pName                           = "main";
        stages.emplace_back(pipelineShaderStageCreateInfo);
        modules.emplace_back(shaderModule);

        // Shader group
        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCreateInfo = {};
        shaderGroupCreateInfo.anyHitShader                         = VK_SHADER_UNUSED_KHR;
        shaderGroupCreateInfo.closestHitShader                     = VK_SHADER_UNUSED_KHR;
        shaderGroupCreateInfo.generalShader                        = VK_SHADER_UNUSED_KHR;
        shaderGroupCreateInfo.intersectionShader                   = VK_SHADER_UNUSED_KHR;

        shaderGroupCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

        if (stageFlag == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
            shaderGroupCreateInfo.type             = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            shaderGroupCreateInfo.closestHitShader = static_cast<uint32_t>(stages.size() - 1);
        }
        else {
            shaderGroupCreateInfo.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroupCreateInfo.generalShader = static_cast<uint32_t>(stages.size() - 1);
        }

        shaderGroups.emplace_back(shaderGroupCreateInfo);
    }

    shader->CreateReflection();
}

void PipelineRayTracing::CreateDescriptorLayout()
{
    const auto& descriptorSetLayoutBindings = shader->GetDescriptorSetLayouts();
    const auto& descriptorSetInfos          = shader->GetDescriptorSetInfos();

    for (const auto& [setIndex, LayoutBindingsForSet] : descriptorSetLayoutBindings) {
        const auto& descriptorSetInfo = descriptorSetInfos.at(setIndex);

        if (descriptorSetInfo.isBindless)
            CreateBindlessDescriptorLayout(setIndex, LayoutBindingsForSet);
        else
            CreateNormalDescriptorLayout(setIndex, LayoutBindingsForSet);
    }
}

void PipelineRayTracing::CreateBindlessDescriptorLayout(uint32_t                                         setIndex,
                                                        const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    if (descriptorSetBindlessLayouts.count(setIndex) == 0)
        descriptorSetBindlessLayouts[setIndex] = VK_NULL_HANDLE;
    else
        Log::Error("This Bindless descriptorSetLayout have exist, Muliple create setLayout!");

    uint32_t bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());

    VkDescriptorBindingFlagsEXT flag = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                                       VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
    std::vector<VkDescriptorBindingFlagsEXT> flags(bindingCount, flag);

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags{};
    bindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    bindingFlags.bindingCount  = bindingCount;
    bindingFlags.pBindingFlags = flags.data();

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags =
        pushDescriptors ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
                        : VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
    descriptorSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBindings.data();
    descriptorSetLayoutCreateInfo.pNext        = &bindingFlags;

    Graphics::CheckVk(vkCreateDescriptorSetLayout(*logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetBindlessLayouts[setIndex]));
}

void PipelineRayTracing::CreateNormalDescriptorLayout(uint32_t setIndex, const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    if (descriptorSetNormalLayouts.count(setIndex) == 0)
        descriptorSetNormalLayouts[setIndex] = VK_NULL_HANDLE;
    else
        Log::Error("This Normal descriptorSetLayout have exist, Muliple create setLayout!");

    uint32_t bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags                           = pushDescriptors ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
    descriptorSetLayoutCreateInfo.bindingCount                    = bindingCount;
    descriptorSetLayoutCreateInfo.pBindings                       = descriptorSetLayoutBindings.data();

    Graphics::CheckVk(vkCreateDescriptorSetLayout(*logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetNormalLayouts[setIndex]));
}

void PipelineRayTracing::CreateDescriptorPool()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto& descriptorPools = shader->GetDescriptorPools();

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    descriptorPoolCreateInfo.maxSets       = 8192;   // 16384;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPools.size());
    descriptorPoolCreateInfo.pPoolSizes    = descriptorPools.data();
    Graphics::CheckVk(vkCreateDescriptorPool(*logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
}

void PipelineRayTracing::CreatePipelineLayout()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto pushConstantRanges = shader->GetPushConstantRanges();

    std::vector<VkDescriptorSetLayout> descriptorSetLayoutsData;
    for (const auto& [setIndex, descriptorSetLayout] : descriptorSetNormalLayouts) descriptorSetLayoutsData.push_back(descriptorSetLayout);
    for (const auto& [setIndex, descriptorSetLayout] : descriptorSetBindlessLayouts) descriptorSetLayoutsData.push_back(descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount             = descriptorSetLayoutsData.size();
    pipelineLayoutCreateInfo.pSetLayouts                = descriptorSetLayoutsData.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount     = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pPushConstantRanges        = pushConstantRanges.data();
    Graphics::CheckVk(vkCreatePipelineLayout(*logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void PipelineRayTracing::CreatePipelineRayTracing()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
    rayTracingPipelineCreateInfo.sType                             = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCreateInfo.stageCount                        = static_cast<uint32_t>(stages.size());
    rayTracingPipelineCreateInfo.pStages                           = stages.data();
    rayTracingPipelineCreateInfo.groupCount                        = static_cast<uint32_t>(shaderGroups.size());
    rayTracingPipelineCreateInfo.pGroups                           = shaderGroups.data();
    rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth      = 1;
    rayTracingPipelineCreateInfo.layout                            = pipelineLayout;
    Graphics::CheckVk(
        vkCreateRayTracingPipelinesKHR(*logicalDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &pipeline));
}

void PipelineRayTracing::CreateShaderBindingTable()
{
    uint32_t handleCount = shaderGroups.size();

    uint32_t handleSize                 = Graphics::Get()->GetPhysicalDevice()->GetRayTracingProperties().shaderGroupHandleSize;
    uint32_t shaderGroupHandleAlignment = Graphics::Get()->GetPhysicalDevice()->GetRayTracingProperties().shaderGroupHandleAlignment;
    uint32_t shaderGroupBaseAlignment   = Graphics::Get()->GetPhysicalDevice()->GetRayTracingProperties().shaderGroupBaseAlignment;

    uint32_t handleSizeAligned = Maths::AlignUp(handleSize, shaderGroupHandleAlignment);

    rgenRegion.stride = Maths::AlignUp(handleSizeAligned, shaderGroupBaseAlignment);
    rgenRegion.size   = rgenRegion.stride;
    missRegion.stride = handleSizeAligned;
    missRegion.size   = Maths::AlignUp(missShaderCount * handleSizeAligned, shaderGroupBaseAlignment);
    hitRegion.stride  = handleSizeAligned;
    hitRegion.size    = Maths::AlignUp(hitShaderCount * handleSizeAligned, shaderGroupBaseAlignment);
    callRegion.stride = handleSizeAligned;
    callRegion.size   = Maths::AlignUp(callableShaderCount * handleSizeAligned, shaderGroupBaseAlignment);

    uint32_t             dataSize = handleCount * handleSize;
    std::vector<uint8_t> shaderHandleStorage(dataSize);
    Graphics::CheckVk(
        vkGetRayTracingShaderGroupHandlesKHR(*Graphics::Get()->GetLogicalDevice(), pipeline, 0, handleCount, dataSize, shaderHandleStorage.data()));

    uint32_t sbtSize = rgenRegion.size + missRegion.size + hitRegion.size + callRegion.size;
    SBTBuffer        = std::make_unique<Buffer>(sbtSize,
                                         VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    rgenRegion.deviceAddress = SBTBuffer->GetDeviceAddress();
    missRegion.deviceAddress = rgenRegion.deviceAddress + rgenRegion.size;
    hitRegion.deviceAddress  = missRegion.deviceAddress + missRegion.size;
    callRegion.deviceAddress = hitRegion.deviceAddress + hitRegion.size;

    auto getHandle = [&](int i) { return shaderHandleStorage.data() + i * handleSize; };

    void* mapped;
    SBTBuffer->MapMemory(&mapped);
    uint32_t handleIdx = 0;

    std::memcpy(mapped, getHandle(handleIdx++), handleSize);
    for (uint32_t i = 0; i < missShaderCount; i++) {
        std::memcpy(static_cast<uint8_t*>(mapped) + rgenRegion.size + i * handleSizeAligned, getHandle(handleIdx++), handleSize);
    }
    for (uint32_t i = 0; i < hitShaderCount; i++) {
        std::memcpy(static_cast<uint8_t*>(mapped) + rgenRegion.size + missRegion.size + i * handleSizeAligned, getHandle(handleIdx++), handleSize);
    }
    for (uint32_t i = 0; i < callableShaderCount; i++) {
        std::memcpy(static_cast<uint8_t*>(mapped) + rgenRegion.size + missRegion.size + hitRegion.size + i * handleSizeAligned,
                    getHandle(handleIdx++),
                    handleSize);
    }

    SBTBuffer->UnmapMemory();
}

}   // namespace MapleLeaf