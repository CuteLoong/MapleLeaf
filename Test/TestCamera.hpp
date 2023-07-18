#include "Camera.hpp"
#include "Devices.hpp"
#include "Inputs.hpp"
#include "Scenes.hpp"
#include "glm/trigonometric.hpp"

namespace MapleLeaf {
class TestCamera : public Camera
{
public:
    TestCamera() {
        position = glm::vec3(0.0f, 0.0f, 2.0f);
        rotation = glm::vec3(0.0f, 0.0f, glm::radians(270.0f));
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

            position.x += -(velocity.z * std::sin(rotation.y) + velocity.x * std::cos(rotation.y));
            position.y += velocity.y;
		    position.z += (velocity.z * std::cos(rotation.y) - velocity.x * std::sin(rotation.y));

            forward.x = std::cos(rotation.y);
            forward.y = 0;
            forward.z = std::sin(rotation.y);
        }
        viewMatrix       = glm::lookAt(position, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        projectionMatrix = glm::perspective(GetFieldOfView(), Devices::Get()->GetWindow()->GetAspectRatio(), GetNearPlane(), GetFarPlane());
    }
};
}   // namespace MapleLeaf