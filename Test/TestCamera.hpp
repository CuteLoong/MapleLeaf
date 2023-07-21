#include "Camera.hpp"
#include "Devices.hpp"
#include "Inputs.hpp"
#include "Scenes.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

namespace MapleLeaf {
class TestCamera : public Camera
{
public:
    TestCamera() {
        position = glm::vec3(0.0f, 0.0f, 2.0f);
        rotation = glm::vec3(0.0f, glm::radians(-90.0f), 0.0f);
    }

    void Start() override {}
    void Update() override
    {
        auto delta = Engine::Get()->GetDelta().AsSeconds();
        glm::vec3 forward = glm::vec3(0.0f);

        if(!Scenes::Get()->GetScene()->IsPaused()) {
            auto positionDelta = Inputs::Get()->GetPositionDelta();
            auto rotationDelta = Inputs::Get()->GetRotationDelta();
            Inputs::Get()->ResetPositionDelta();
            Inputs::Get()->ResetRotationDelta();

            velocity = 3.0f * delta * positionDelta;

            rotation.y += rotationDelta.x;
            rotation.x += rotationDelta.y;
            rotation.x = std::clamp(rotation.x, glm::radians(90.0f), glm::radians(270.0f));

            forward.x = std::cos(rotation.y);
            forward.y = 0;
            forward.z = std::sin(rotation.y);

            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::cross(forward, up);

            position += velocity.z * forward;
            position += velocity.x * right;
            position += velocity.y * up;
        }
        viewMatrix       = glm::lookAt(position, position + forward, glm::vec3(0.0, 1.0, 0.0));
        projectionMatrix = glm::perspective(GetFieldOfView(), Devices::Get()->GetWindow()->GetAspectRatio(), GetNearPlane(), GetFarPlane());
    }
};
}   // namespace MapleLeaf