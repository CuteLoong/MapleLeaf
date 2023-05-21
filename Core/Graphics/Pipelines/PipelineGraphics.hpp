#pragma once

#include "Image2d.hpp"
#include "ImageDepth.hpp"
#include "Pipeline.hpp"
#include "RenderStage.hpp"
#include "glm/glm.hpp"
#include "vulkan/vulkan_core.h"
#include <filesystem>
#include <vector>


namespace MapleLeaf {
class ImageDepth;
class Image2d;

class PipelineGraphics : public Pipeline
{
public:
    enum class Mode
    {
        Polygon,
        MRT
    };

    enum class Depth
    {
        None      = 0,
        Read      = 1,
        Write     = 2,
        ReadWrite = Read | Write
    };
    PipelineGraphics(Stage stage, std::vector<std::filesystem::path> shaderStages, std::vector<Shader::VertexInput> vertexInputs,
                     std::vector<Shader::Define> defines = {}, Mode mode = Mode::Polygon, Depth depth = Depth::ReadWrite,
                     VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
                     VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE, bool pushDescriptors = false);
    ~PipelineGraphics();

    /**
     * Gets the depth stencil used in a stage.
     * @param stage The stage to get values from, if not provided the pipelines stage will be used.
     * @return The depth stencil that is found.
     */
    const ImageDepth* GetDepthStencil(const std::optional<uint32_t>& stage = std::nullopt) const;

    /**
     * Gets a image used in a stage by the index given to it in the renderpass.
     * @param index The renderpass Image index.
     * @param stage The stage to get values from, if not provided the pipelines stage will be used.
     * @return The image that is found.
     */
    const Image2d* GetImage(uint32_t index, const std::optional<uint32_t>& stage = std::nullopt) const;

    /**
     * Gets the render stage viewport.
     * @param stage The stage to get values from, if not provided the pipelines stage will be used.
     * @return The the render stage viewport.
     */
    RenderArea GetRenderArea(const std::optional<uint32_t>& stage = std::nullopt) const;

    const Stage&                              GetStage() const { return stage; }
    const std::vector<std::filesystem::path>& GetShaderStages() const { return shaderStages; }
    const std::vector<Shader::VertexInput>&   GetVertexInputs() const { return vertexInputs; }
    const std::vector<Shader::Define>&        GetDefines() const { return defines; }
    Mode                                      GetMode() const { return mode; }
    Depth                                     GetDepth() const { return depth; }
    VkPrimitiveTopology                       GetTopology() const { return topology; }
    VkPolygonMode                             GetPolygonMode() const { return polygonMode; }
    VkCullModeFlags                           GetCullMode() const { return cullMode; }
    VkFrontFace                               GetFrontFace() const { return frontFace; }
    bool                                      IsPushDescriptors() const override { return pushDescriptors; }
    const Shader*                             GetShader() const override { return shader.get(); }
    const VkDescriptorSetLayout&              GetDescriptorSetLayout() const override { return descriptorSetLayout; }
    const VkDescriptorPool&                   GetDescriptorPool() const override { return descriptorPool; }
    const VkPipeline&                         GetPipeline() const override { return pipeline; }
    const VkPipelineLayout&                   GetPipelineLayout() const override { return pipelineLayout; }
    const VkPipelineBindPoint&                GetPipelineBindPoint() const override { return pipelineBindPoint; }

private:
    Stage                              stage;
    std::vector<std::filesystem::path> shaderStages;
    std::vector<Shader::VertexInput>   vertexInputs;
    std::vector<Shader::Define>        defines;
    Mode                               mode;
    Depth                              depth;
    VkPrimitiveTopology                topology;
    VkPolygonMode                      polygonMode;
    VkCullModeFlags                    cullMode;
    VkFrontFace                        frontFace;
    bool                               pushDescriptors;
    std::unique_ptr<Shader>            shader;

    std::vector<VkDynamicState> dynamicStates;

    std::vector<VkShaderModule>                  modules;
    std::vector<VkPipelineShaderStageCreateInfo> stages;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool      descriptorPool      = VK_NULL_HANDLE;

    VkPipeline          pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint pipelineBindPoint;

    VkPipelineVertexInputStateCreateInfo               vertexInputStateCreateInfo = {};
    VkPipelineInputAssemblyStateCreateInfo             inputAssemblyState         = {};
    VkPipelineRasterizationStateCreateInfo             rasterizationState         = {};
    std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates;
    VkPipelineColorBlendStateCreateInfo                colorBlendState   = {};
    VkPipelineDepthStencilStateCreateInfo              depthStencilState = {};
    VkPipelineViewportStateCreateInfo                  viewportState     = {};
    VkPipelineMultisampleStateCreateInfo               multisampleState  = {};
    VkPipelineDynamicStateCreateInfo                   dynamicState      = {};
    VkPipelineTessellationStateCreateInfo              tessellationState = {};

    void CreateShaderProgram();
    void CreateDescriptorLayout();
    void CreateDescriptorPool();
    void CreatePipelineLayout();
    void CreateAttributes();
    void CreatePipeline();
    void CreatePipelinePolygon();
    void CreatePipelineMrt();
};

class PipelineGraphicsCreate
{
public:
private:
    std::vector<std::filesystem::path> shaderStages;
    std::vector<Shader::VertexInput>   vertexInputs;
    std::vector<Shader::Define>        defines;

    PipelineGraphics::Mode  mode;
    PipelineGraphics::Depth depth;
    VkPrimitiveTopology     topology;
    VkPolygonMode           polygonMode;
    VkCullModeFlags         cullMode;
    VkFrontFace             frontFace;
    bool                    pushDescriptors;
};
}   // namespace MapleLeaf