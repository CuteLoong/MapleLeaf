#pragma once

#include "Bitmap.hpp"
#include "Buffer.hpp"
#include "Devices.hpp"
#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Imgui.h"
#include "Module.hpp"
#include "Shader.hpp"
#include "imgui_impl_glfw.h"

namespace MapleLeaf {
class Imgui : public Module::Registrar<Imgui>
{
    inline static const bool Registered = Register(Stage::Render, Requires<Devices, Graphics>());

public:
    Imgui();
    ~Imgui();

    void Update() override;

    bool cmdRender(const CommandBuffer& commandBuffer);

    const glm::vec2& GetScale() const { return scale; }
    const glm::vec2& GetTranslate() const { return translate; }

    const Image2d* GetFontImage() const { return fontImage.get(); }

    bool GetImguiCursorState() const { return ImGui::GetIO().WantCaptureMouse; }

    static Shader::VertexInput GetVertexInput(uint32_t baseBinding = 0)
    {
        std::vector<VkVertexInputBindingDescription>   bindingDescriptions   = {{baseBinding, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX}};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            {0, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)},
            {1, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)},
            {2, baseBinding, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)}};
        return {bindingDescriptions, attributeDescriptions};
    }

private:
    std::unique_ptr<Buffer>  vertexBuffer;
    std::unique_ptr<Buffer>  indexBuffer;
    std::unique_ptr<Image2d> fontImage;

    glm::vec2 scale, translate;   // for shader

    void UpdateDrawData();
    void SetImguiDrawData(ImDrawData* imDrawData);
    void ImguiNewFrame();
};
}   // namespace MapleLeaf