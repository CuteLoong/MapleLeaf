#pragma once

#include "DescriptorHandler.hpp"
#include "MaterialPipeline.hpp"
#include "StreamFactory.hpp"
#include "UniformHandler.hpp"
#include "glm/fwd.hpp"

namespace MapleLeaf {
class Material : public StreamFactory<Material>
{
public:
    enum class TextureSlot
    {
        BaseColor,
        Material,   // metalic roughness's image
        Normal,
    };

    virtual ~Material() = default;

    virtual void CreatePipeline(const Shader::VertexInput& vertexInput) = 0;

    /**
     * Used to update the main uniform handler used in a material.
     * A material can defined it's own uniforms and push them via {@link Material#PushDescriptors()}.
     * @param uniformObject The uniform handler to update.
     */
    virtual void PushUniforms(UniformHandler& uniformObject, const glm::mat4* transform) = 0;

    /**
     * Used to update a descriptor set containing descriptors used in this materials shader.
     * @param descriptorSet The descriptor handler to update.
     */
    virtual void PushDescriptors(DescriptorsHandler& descriptorSet) = 0;

    const std::shared_ptr<MaterialPipeline>& GetPipelineMaterial() const { return pipelineMaterial; }

protected:
    std::shared_ptr<MaterialPipeline> pipelineMaterial;
};

template class TypeInfo<Material>;
}   // namespace MapleLeaf