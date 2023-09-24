#include "Inputs.hpp"
#include "Devices.hpp"
#include "KeyEnums.hpp"
#include "Imgui.hpp"

namespace MapleLeaf {
Inputs::Inputs()
{
    Devices::Get()->GetWindow()->OnMousePosition().connect(boost::bind(&Inputs::ProcessMousePosition, this, boost::placeholders::_1));
    Devices::Get()->GetWindow()->OnMouseButton().connect(
        boost::bind(&Inputs::ProcessMouseButton, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
    Devices::Get()->GetWindow()->OnKey().connect(
        boost::bind(&Inputs::ProcessKeyboard, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
}

void Inputs::Update() {}


void Inputs::ProcessMouseButton(MouseButton mouseButton, InputAction inputAction, InputMod inputMod)
{
    if ((mouseButton == MouseButton::_1) && (inputAction == InputAction::Press)) cusorLeftPress = true;
    if ((mouseButton == MouseButton::_1) && (inputAction == InputAction::Release)) cusorLeftPress = false;
}

void Inputs::ProcessKeyboard(Key key, InputAction inputAction, InputMod inputMod)
{
    bool moveButtonPressed = inputAction == InputAction::Repeat || inputAction == InputAction::Press;
    if ((key == Key::W) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.z += 1.0;
    if ((key == Key::S) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.z -= 1.0;
    if ((key == Key::A) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.x -= 1.0;
    if ((key == Key::D) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.x += 1.0;
    if ((key == Key::E) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.y += 1.0;
    if ((key == Key::Q) && moveButtonPressed && (inputMod == InputMod::None)) positionDelta.y -= 1.0;
}

void Inputs::ProcessMousePosition(glm::vec2 value)
{
    if (cusorLeftPress && !Imgui::Get()->GetImguiCursorState()) rotationDelta -= Devices::Get()->GetWindow()->GetMousePositionDelta();
}

}   // namespace MapleLeaf
