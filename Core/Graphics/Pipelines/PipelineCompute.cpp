#include "PipelineCompute.hpp"

#include "Files.hpp"
#include "Graphics.hpp"


namespace MapleLeaf {
PipelineCompute::PipelineCompute(std::filesystem::path shaderStage, std::vector<Shader::Define> defines, bool pushDescriptors)
    : shaderStage(std::move(shaderStage))
    , defines(std::move(defines))
    , pushDescriptors(pushDescriptors)
    , shader(std::make_unique<Shader>())
    , pipelineBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE)
{
#ifdef ACID_DEBUG
    auto debugStart = Time::Now();
#endif

    CreateShaderProgram();
    CreateDescriptorLayout();
    CreateDescriptorPool();
    CreatePipelineLayout();
    CreatePipelineCompute();

#ifdef ACID_DEBUG
    Log::Out("Pipeline Compute ", this->shaderStage, " created in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

PipelineCompute::~PipelineCompute()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    vkDestroyShaderModule(*logicalDevice, shaderModule, nullptr);

    for (auto& [setIndex, descriptorSetLayout] : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(*logicalDevice, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(*logicalDevice, descriptorPool, nullptr);
    vkDestroyPipeline(*logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(*logicalDevice, pipelineLayout, nullptr);
}

void PipelineCompute::CmdRender(const CommandBuffer& commandBuffer, const glm::uvec2& extent) const
{
    auto groupCountX = static_cast<uint32_t>(std::ceil(static_cast<float>(extent.x) / static_cast<float>(*shader->GetLocalSizes()[0])));
    auto groupCountY = static_cast<uint32_t>(std::ceil(static_cast<float>(extent.y) / static_cast<float>(*shader->GetLocalSizes()[1])));
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, 1);
}

void PipelineCompute::CreateShaderProgram()
{
    std::stringstream defineBlock;
    for (const auto& [defineName, defineValue] : defines) defineBlock << "#define " << defineName << " " << defineValue << '\n';

    auto fileLoaded = Files::Read(shaderStage);

    if (!fileLoaded) throw std::runtime_error("Could not create pipeline, missing shader stage");

    auto stageFlag    = Shader::GetShaderStage(shaderStage);
    auto shaderModule = shader->CreateShaderModule(shaderStage, *fileLoaded, defineBlock.str(), stageFlag);

    shaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage  = stageFlag;
    shaderStageCreateInfo.module = shaderModule;
    shaderStageCreateInfo.pName  = "main";

    shader->CreateReflection();
}

void PipelineCompute::CreateDescriptorLayout()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto& descriptorSetLayoutBindings = shader->GetDescriptorSetLayouts();
    for (const auto& [setIndex, descriptorSetLayoutBinding] : descriptorSetLayoutBindings) {
        descriptorSetLayouts[setIndex] = VK_NULL_HANDLE;

        VkDescriptorBindingFlagsEXT flags;
        if (descriptorSetLayoutBinding.size() != 1) {
            flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
        }
        else {
            flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
        }
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags{};
        bindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        bindingFlags.bindingCount  = 1;
        bindingFlags.pBindingFlags = &flags;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.flags =
            pushDescriptors ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
                            : VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
        descriptorSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBinding.data();
        descriptorSetLayoutCreateInfo.pNext        = &bindingFlags;

        Graphics::CheckVk(vkCreateDescriptorSetLayout(*logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[setIndex]));
    }
}

void PipelineCompute::CreateDescriptorPool()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto descriptorPools = shader->GetDescriptorPools();

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.maxSets                    = 8192;   // 16384;
    descriptorPoolCreateInfo.poolSizeCount              = static_cast<uint32_t>(descriptorPools.size());
    descriptorPoolCreateInfo.pPoolSizes                 = descriptorPools.data();
    Graphics::CheckVk(vkCreateDescriptorPool(*logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
}

void PipelineCompute::CreatePipelineLayout()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto pushConstantRanges = shader->GetPushConstantRanges();

    std::vector<VkDescriptorSetLayout> descriptorSetLayoutsData;
    for (const auto& [setIndex, descriptorSetLayout] : descriptorSetLayouts) descriptorSetLayoutsData.push_back(descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount             = descriptorSetLayoutsData.size();
    pipelineLayoutCreateInfo.pSetLayouts                = descriptorSetLayoutsData.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount     = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pPushConstantRanges        = pushConstantRanges.data();
    Graphics::CheckVk(vkCreatePipelineLayout(*logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void PipelineCompute::CreatePipelineCompute()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();
    auto pipelineCache = Graphics::Get()->GetPipelineCache();

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage                       = shaderStageCreateInfo;
    pipelineCreateInfo.layout                      = pipelineLayout;
    pipelineCreateInfo.basePipelineHandle          = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex           = -1;
    vkCreateComputePipelines(*logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline);
}
}   // namespace MapleLeaf