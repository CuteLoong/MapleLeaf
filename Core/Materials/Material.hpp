#pragma once

#include "MaterialPipeline.hpp"
#include "StreamFactory.hpp"


namespace MapleLeaf {
class Material : public StreamFactory<Material>
{
public:
    virtual ~Material() = default;

    virtual void CreatePipeline(const Shader::VertexInput& vertexInput, bool animated) = 0;

    

    const std::shared_ptr<MaterialPipeline>& GetPipelineMaterial() const { return pipelineMaterial; }

protected:
    std::shared_ptr<MaterialPipeline> pipelineMaterial;
};

template class TypeInfo<Material>;
}   // namespace MapleLeaf