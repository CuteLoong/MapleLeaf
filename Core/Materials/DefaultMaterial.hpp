#pragma once

#include "Color.hpp"
#include "Image2d.hpp"
#include "Material.hpp"

namespace MapleLeaf {
class DefaultMaterial : public Material::Registrar<DefaultMaterial>
{
    inline static const bool Registered = Register("default");

public:
    explicit DefaultMaterial(const Color& baseDiffuse = Color::White, std::shared_ptr<Image2d> imageDiffuse = nullptr, float metallic = 0.0f,
                             float roughness = 0.0f, std::shared_ptr<Image2d> imageMaterial = nullptr, std::shared_ptr<Image2d> imageNormal = nullptr,
                             bool castsShadows = true, bool ignoreLighting = false, bool ignoreFog = false);

    void CreatePipeline(const Shader::VertexInput& vertexInput) override;
    void PushUniforms(UniformHandler& uniformObject, const Transform* transform) override;
    void PushDescriptors(DescriptorsHandler& descriptorSet) override;

    const Color& GetBaseDiffuse() const { return baseDiffuse; }
    void         SetBaseDiffuse(const Color& baseDiffuse) { this->baseDiffuse = baseDiffuse; }

    const std::shared_ptr<Image2d>& GetImageDiffuse() const { return imageDiffuse; }
    void                            SetImageDiffuse(const std::shared_ptr<Image2d>& imageDiffuse) { this->imageDiffuse = imageDiffuse; }

    float GetMetallic() const { return metallic; }
    void  SetMetallic(float metallic) { this->metallic = metallic; }

    float GetRoughness() const { return roughness; }
    void  SetRoughness(float roughness) { this->roughness = roughness; }

    const std::shared_ptr<Image2d>& GetImageMaterial() const { return imageMaterial; }
    void                            SetImageMaterial(const std::shared_ptr<Image2d>& imageMaterial) { this->imageMaterial = imageMaterial; }

    const std::shared_ptr<Image2d>& GetImageNormal() const { return imageNormal; }
    void                            SetImageNormal(const std::shared_ptr<Image2d>& imageNormal) { this->imageNormal = imageNormal; }

    bool IsCastsShadows() const { return castsShadows; }
    void SetCastsShadows(bool castsShadows) { this->castsShadows = castsShadows; }

    bool IsIgnoringLighting() const { return ignoreLighting; }
    void SetIgnoreLighting(bool ignoreLighting) { this->ignoreLighting = ignoreLighting; }

    bool IsIgnoringFog() const { return ignoreFog; }
    void SetIgnoreFog(bool ignoreFog) { this->ignoreFog = ignoreFog; }

private:
    std::vector<Shader::Define> GetDefines() const;

    Color                    baseDiffuse;
    std::shared_ptr<Image2d> imageDiffuse;

    float                    metallic;
    float                    roughness;
    std::shared_ptr<Image2d> imageMaterial;
    std::shared_ptr<Image2d> imageNormal;

    bool castsShadows;
    bool ignoreLighting;
    bool ignoreFog;
};
}   // namespace MapleLeaf