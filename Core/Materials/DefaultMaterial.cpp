#include "DefaultMaterial.hpp"
#include "Transform.hpp"
#include <string>

namespace MapleLeaf {
DefaultMaterial::DefaultMaterial(const Color& baseDiffuse, std::shared_ptr<Image2d> imageDiffuse, float metallic, float roughness,
                                 std::shared_ptr<Image2d> imageMaterial, std::shared_ptr<Image2d> imageNormal, bool castsShadows, bool ignoreLighting,
                                 bool ignoreFog)
    : baseDiffuse(baseDiffuse)
    , imageDiffuse(std::move(imageDiffuse))
    , metallic(metallic)
    , roughness(roughness)
    , imageMaterial(std::move(imageMaterial))
    , imageNormal(std::move(imageNormal))
    , castsShadows(castsShadows)
    , ignoreLighting(ignoreLighting)
    , ignoreFog(ignoreFog)
{}

void DefaultMaterial::CreatePipeline(const Shader::VertexInput& vertexInput)
{
    // stage-0 is shadow pass
    pipelineMaterial = MaterialPipeline::Create(
        {1, 0}, {{"Shader/Defaults/Default.vert", "Shader/Defaults/Default.frag"}, {vertexInput}, GetDefines(), PipelineGraphics::Mode::MRT});
}

void DefaultMaterial::PushUniforms(UniformHandler& uniformObject, const Transform* transform)
{
    if (transform) {
        uniformObject.Push("transform", transform->GetWorldMatrix());
        uniformObject.Push("prevTransform", transform->GetPrevWorldMatrix());
    }

    uniformObject.Push("baseDiffuse", baseDiffuse);
    uniformObject.Push("metallic", metallic);
    uniformObject.Push("roughness", roughness);
    uniformObject.Push("ignoreFog", static_cast<float>(ignoreFog));
    uniformObject.Push("ignoreLighting", static_cast<float>(ignoreLighting));
}

void DefaultMaterial::PushDescriptors(DescriptorsHandler& descriptorSet)
{
    descriptorSet.Push("samplerDiffuse", imageDiffuse);
    descriptorSet.Push("samplerMaterial", imageMaterial);
    descriptorSet.Push("samplerNormal", imageNormal);
}

std::vector<Shader::Define> DefaultMaterial::GetDefines() const
{
    return {
        {"DIFFUSE_MAPPING", std::to_string(static_cast<uint32_t>((imageDiffuse != nullptr)))},
        {"MATERIAL_MAPPING", std::to_string(static_cast<uint32_t>(imageMaterial != nullptr))},
        {"NORMAL_MAPPING", std::to_string(static_cast<uint32_t>(imageNormal != nullptr))},
    };
}
}   // namespace MapleLeaf