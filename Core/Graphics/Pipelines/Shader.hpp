#pragma once

#include "volk.h"
#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace glslang {
class TProgram;
class TType;
}   // namespace glslang

namespace MapleLeaf {
class Shader
{
public:
    /**
     * A define added to the start of a shader, first value is the define name and second is the value to be set.
     */
    using Define = std::pair<std::string, std::string>;
    class VertexInput
    {
    public:
        VertexInput(std::vector<VkVertexInputBindingDescription>   bindingDescriptions   = {},
                    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {})
            : bindingDescriptions(std::move(bindingDescriptions))
            , attributeDescriptions(std::move(attributeDescriptions))
        {}

        bool operator<(const VertexInput& rhs) const { return bindingDescriptions.front().binding < rhs.bindingDescriptions.front().binding; }

        const std::vector<VkVertexInputBindingDescription>&   GetBindingDescriptions() const { return bindingDescriptions; }
        const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

    private:
        uint32_t                                       binding = 0;
        std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    class Uniform
    {
        friend class Shader;

    public:
        explicit Uniform(uint32_t set = -1, int32_t binding = -1, int32_t offset = -1, int32_t size = -1, int32_t glType = -1, bool readOnly = false,
                         bool writeOnly = false, bool bindless = false, VkShaderStageFlags stageFlags = 0)
            : set(set)
            , binding(binding)
            , offset(offset)
            , size(size)
            , glType(glType)
            , readOnly(readOnly)
            , writeOnly(writeOnly)
            , bindless(bindless)
            , stageFlags(stageFlags)
        {}

        uint32_t           GetSet() const { return set; }
        int32_t            GetBinding() const { return binding; }
        int32_t            GetOffset() const { return offset; }
        int32_t            GetSize() const { return size; }
        int32_t            GetGlType() const { return glType; }
        bool               IsReadOnly() const { return readOnly; }
        bool               IsWriteOnly() const { return writeOnly; }
        bool               IsBindless() const { return bindless; }
        VkShaderStageFlags GetStageFlags() const { return stageFlags; }

        bool operator==(const Uniform& rhs) const
        {
            return binding == rhs.binding && offset == rhs.offset && size == rhs.size && glType == rhs.glType && readOnly == rhs.readOnly &&
                   writeOnly == rhs.writeOnly && stageFlags == rhs.stageFlags;
        }

        bool operator!=(const Uniform& rhs) const { return !operator==(rhs); }

    private:
        uint32_t           set;
        int32_t            binding;
        int32_t            offset;
        int32_t            size;
        int32_t            glType;
        bool               readOnly;
        bool               writeOnly;
        bool               bindless;
        VkShaderStageFlags stageFlags;
    };

    class UniformBlock
    {
        friend class Shader;

    public:
        enum class Type
        {
            None,
            Uniform,
            Storage,
            Push
        };
        explicit UniformBlock(uint32_t set = -1, int32_t binding = -1, int32_t size = -1, VkShaderStageFlags stageFlags = 0,
                              Type type = Type::Uniform, bool bindless = false)
            : set(set)
            , binding(binding)
            , size(size)
            , stageFlags(stageFlags)
            , type(type)
            , bindless(bindless)
        {}

        uint32_t                              GetSet() const { return set; }
        int32_t                               GetBinding() const { return binding; }
        int32_t                               GetSize() const { return size; }
        VkShaderStageFlags                    GetStageFlags() const { return stageFlags; }
        Type                                  GetType() const { return type; }
        bool                                  IsBindless() const { return bindless; }
        const std::map<std::string, Uniform>& GetUniforms() const { return uniforms; }

        std::optional<Uniform> GetUniform(const std::string& name) const
        {
            auto it = uniforms.find(name);

            if (it == uniforms.end()) {
                return std::nullopt;
            }

            return it->second;
        }

        bool operator==(const UniformBlock& rhs) const
        {
            return binding == rhs.binding && size == rhs.size && stageFlags == rhs.stageFlags && type == rhs.type && uniforms == rhs.uniforms;
        }

        bool operator!=(const UniformBlock& rhs) const { return !operator==(rhs); }

    private:
        uint32_t                       set;
        int32_t                        binding;
        int32_t                        size;
        VkShaderStageFlags             stageFlags;
        Type                           type;
        bool                           bindless;
        std::map<std::string, Uniform> uniforms;
    };

    class Attribute
    {
        friend class Shader;

    public:
        explicit Attribute(int32_t set = -1, int32_t location = -1, int32_t size = -1, int32_t glType = -1)
            : set(set)
            , location(location)
            , size(size)
            , glType(glType)
        {}

        int32_t GetSet() const { return set; }
        int32_t GetLocation() const { return location; }
        int32_t GetSize() const { return size; }
        int32_t GetGlType() const { return glType; }

        bool operator==(const Attribute& rhs) const { return set == rhs.set && location == rhs.location && size == rhs.size && glType == rhs.glType; }

        bool operator!=(const Attribute& rhs) const { return !operator==(rhs); }

    private:
        int32_t set;
        int32_t location;
        int32_t size;
        int32_t glType;
    };

    // class Constant
    // {
    //     friend class Shader;

    // public:
    //     explicit Constant(int32_t binding = -1, int32_t size = -1, VkShaderStageFlags stageFlags = 0, int32_t glType = -1)
    //         : binding(binding)
    //         , size(size)
    //         , stageFlags(stageFlags)
    //         , glType(glType)
    //     {}

    //     int32_t            GetBinding() const { return binding; }
    //     int32_t            GetSize() const { return size; }
    //     VkShaderStageFlags GetStageFlags() const { return stageFlags; }
    //     int32_t            GetGlType() const { return glType; }

    //     bool operator==(const Constant& rhs) const
    //     {
    //         return binding == rhs.binding && size == rhs.size && stageFlags == rhs.stageFlags && glType == rhs.glType;
    //     }

    //     bool operator!=(const Constant& rhs) const { return !operator==(rhs); }

    // private:
    //     int32_t            binding;
    //     int32_t            size;
    //     VkShaderStageFlags stageFlags;
    //     int32_t            glType;
    // };

    struct DescriptorSetInfo
    {
        bool     isBindless;
        uint32_t bindingCount;

        DescriptorSetInfo(bool isBindless = false, uint32_t bindingCount = 0)
            : isBindless(isBindless)
            , bindingCount(bindingCount)
        {}
    };

    Shader();

    bool                                                        ReportedNotFound(const std::string& name, bool reportIfFound) const;
    static VkFormat                                             GlTypeToVk(int32_t type);
    std::pair<std::optional<uint32_t>, std::optional<uint32_t>> GetDescriptorLocation(const std::string& name) const;
    std::optional<uint32_t>                                     GetDescriptorSize(const std::string& name) const;
    std::optional<Uniform>                                      GetUniform(const std::string& name) const;
    std::optional<UniformBlock>                                 GetUniformBlock(const std::string& name) const;
    std::optional<Attribute>                                    GetAttribute(const std::string& name) const;
    std::vector<VkPushConstantRange>                            GetPushConstantRanges() const;

    std::optional<VkDescriptorType> GetDescriptorType(uint32_t setIndex, uint32_t location) const;
    static VkShaderStageFlagBits    GetShaderStage(const std::filesystem::path& filename);
    VkShaderModule CreateShaderModule(const std::filesystem::path& moduleName, const std::string& moduleCode, const std::string& preamble,
                                      VkShaderStageFlags moduleFlag);
    void           CreateReflection();

    const std::filesystem::path&                                         GetName() const { return stages.back(); }
    const std::map<std::string, Uniform>&                                GetUniforms() const { return uniforms; };
    const std::map<std::string, UniformBlock>&                           GetUniformBlocks() const { return uniformBlocks; };
    const std::map<std::string, Attribute>&                              GetAttributes() const { return attributes; };
    const std::array<std::optional<uint32_t>, 3>&                        GetLocalSizes() const { return localSizes; }
    const std::vector<VkDescriptorPoolSize>&                             GetDescriptorPools() const { return descriptorPools; }
    const std::map<uint32_t, uint32_t>&                                  GetLastDescriptorBinding() const { return lastDescriptorBinding; }
    const std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& GetDescriptorSetLayouts() const { return descriptorSetLayouts; }
    const std::map<uint32_t, DescriptorSetInfo>&                         GetDescriptorSetInfos() const { return descriptorSetInfos; }

    // const std::vector<VkVertexInputAttributeDescription>&               GetAttributeDescriptions() const { return attributeDescriptions; }
    // const std::map<std::string, Constant>&                GetConstants() const { return constants; };

private:
    std::vector<std::filesystem::path>  stages;
    std::map<std::string, Uniform>      uniforms;
    std::map<std::string, UniformBlock> uniformBlocks;
    std::map<std::string, Attribute>    attributes;
    // std::map<std::string, Constant>     constants;

    std::array<std::optional<uint32_t>, 3> localSizes;

    std::vector<VkDescriptorPoolSize> descriptorPools;

    // Vector's index is setIndex, value is DescriptorInfo about this descriptor set.
    std::map<uint32_t, std::map<std::string, uint32_t>>           descriptorLocations;
    std::map<uint32_t, std::map<std::string, uint32_t>>           descriptorSizes;
    std::map<uint32_t, std::map<uint32_t, VkDescriptorType>>      descriptorTypes;
    std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayouts;
    std::map<uint32_t, uint32_t>                                  lastDescriptorBinding;
    std::map<uint32_t, DescriptorSetInfo>                         descriptorSetInfos;
    // std::vector<VkVertexInputAttributeDescription>         attributeDescriptions;

    mutable std::vector<std::string> notFoundNames;

    static void    IncrementDescriptorPool(std::map<VkDescriptorType, uint32_t>& descriptorPoolCounts, VkDescriptorType type);
    void           LoadUniformBlock(const glslang::TProgram& program, VkShaderStageFlags stageFlag, int32_t i);
    void           LoadUniform(const glslang::TProgram& program, VkShaderStageFlags stageFlag, int32_t i);
    void           LoadAttribute(const glslang::TProgram& program, VkShaderStageFlags stageFlag, int32_t i);
    static int32_t ComputeSize(const glslang::TType* ttype);
};
}   // namespace MapleLeaf