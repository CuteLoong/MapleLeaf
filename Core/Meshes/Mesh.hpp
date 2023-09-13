#pragma once

#include "Component.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Vertex.hpp"

namespace MapleLeaf {
class Mesh : public Component::Registrar<Mesh>
{
    inline static const bool Registered = Register("mesh");

public:
    enum class UpdateStatus
    {
        None,
        MeshAlter   // model or materail updated
    };

    /**
     * Creates a new mesh component.
     * @param model The model to use in this mesh.
     * @param material The material to render this mesh with.
     */
    explicit Mesh(std::shared_ptr<Model> model = nullptr, std::shared_ptr<Material> material = nullptr);

    void Start() override;
    void Update() override;

    bool CmdRender(const CommandBuffer& commandBuffer, UniformHandler& uniformScene, const Pipeline::Stage& pipelineStage);

    static Shader::VertexInput GetVertexInput(uint32_t binding = 0) { return Vertex3D::GetVertexInput(binding); }

    const std::shared_ptr<Model> GetModel() const { return model; }
    void                         SetModel(const std::shared_ptr<Model>& model) { this->model = model; }

    const std::shared_ptr<Material> GetMaterial() const { return material; }
    void                            SetMaterial(std::shared_ptr<Material>& material);

    UpdateStatus GetUpdateStatus() const { return updateStatus; }

    bool operator<(const Mesh& rhs) const;
    bool operator>(const Mesh& rhs) const;

private:
    std::shared_ptr<Model>    model;
    std::shared_ptr<Material> material;

    DescriptorsHandler descriptorSet;
    UniformHandler     uniformObject;

    UpdateStatus updateStatus;
};
}   // namespace MapleLeaf