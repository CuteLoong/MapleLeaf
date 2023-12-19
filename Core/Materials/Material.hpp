#pragma once

#include "DescriptorHandler.hpp"
#include "MaterialPipeline.hpp"
#include "Resource.hpp"
#include "StreamFactory.hpp"
#include "Transform.hpp"
#include "UniformHandler.hpp"
#include "glm/fwd.hpp"

namespace MapleLeaf {
class Material : public StreamFactory<Material>, public Resource
{
public:
    enum class TextureSlot
    {
        BaseColor,
        Material,   // metalic roughness's image
        Normal,
    };
    std::type_index GetTypeIndex() const override { return typeid(Material); }

    virtual ~Material() = default;

    virtual void CreatePipeline(const Shader::VertexInput& vertexInput) = 0;

    /**
     * Used to update the main uniform handler used in a material.
     * A material can defined it's own uniforms and push them via {@link Material#PushDescriptors()}.
     * @param uniformObject The uniform handler to update.
     */
    virtual void PushUniforms(UniformHandler& uniformObject, const Transform* transform) = 0;

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