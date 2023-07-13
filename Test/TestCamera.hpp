#include "Camera.hpp"
#include "Devices.hpp"

namespace MapleLeaf {
class TestCamera : public Camera
{
public:
    TestCamera() {}

    void Start() override {}
    void Update() override
    {
        viewMatrix       = glm::lookAt(position, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        projectionMatrix = glm::perspective(GetFieldOfView(), Devices::Get()->GetWindow()->GetAspectRatio(), GetNearPlane(), GetFarPlane());
    }
};
}   // namespace MapleLeaf