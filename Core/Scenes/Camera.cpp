#include "Camera.hpp"
#include "Engine.hpp"
#include "Inputs.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
void Camera::Start() {
    UpdateByTransform();
}
void Camera::Update()
{
    const auto& transform = GetEntity()->GetComponent<Transform>();
    if (transform->GetUpdateStatus() == Transform::UpdateStatus::Transformation) {
        UpdateByTransform();
    }
    else {
        UpdateByInput();
    }

    viewMatrix       = glm::lookAt(position, position + forward, up);
    projectionMatrix = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
}

void Camera::UpdateByTransform()
{
    const auto& transform = GetEntity()->GetComponent<Transform>();
    up                    = transform->GetWorldMatrix()[1];
    forward               = -transform->GetWorldMatrix()[2];
    position              = transform->GetWorldMatrix()[3];
    rotation              = transform->GetRotation();
}

void Camera::UpdateByInput()
{
    auto delta = Engine::Get()->GetDelta().AsSeconds();

    if (!Scenes::Get()->GetScene()->IsPaused()) {
        auto positionDelta = Inputs::Get()->GetPositionDelta();
        auto rotationDelta = Inputs::Get()->GetRotationDelta(); // x---yaw y---pitch
        Inputs::Get()->ResetPositionDelta();
        Inputs::Get()->ResetRotationDelta();

        velocity = 5.0f * delta * positionDelta;

        rotation.y += rotationDelta.x * 0.001;
        rotation.x += rotationDelta.y * 0.001;
        rotation.x = std::clamp(rotation.x, glm::radians(-90.0f), glm::radians(90.0f));

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        forward                  = rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

        forward = glm::normalize(forward);

        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        up              = glm::normalize(glm::cross(right, forward));

        position += velocity.z * forward;
        position += velocity.x * right;
        position += velocity.y * up;
    }
}
}   // namespace MapleLeaf