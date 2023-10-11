#include "Camera.hpp"
#include "Engine.hpp"
#include "Inputs.hpp"
#include "Scenes.hpp"

#include "config.h"

namespace MapleLeaf {
Camera::Camera()
    : nearPlane(0.1f)
    , farPlane(100.0f)
    , fieldOfView(glm::radians(60.0f))
    , eyeSeparation(0.08f)
    , up(glm::vec3(0.0f, 1.0f, 0.0f))
    , forward(glm::vec3(0.0f, 0.0f, -1.0f))   // rotation(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f))
    , position(glm::vec3(0.0f, 0.0f, 2.0f))
{
    right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Camera::Start()
{
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

    invViewMatrix       = glm::inverse(viewMatrix);
    invProjectionMatrix = glm::inverse(projectionMatrix);

    stereoViewPosition[0] = glm::vec4(position - right * eyeSeparation / 2.0f, 1.0f);
    stereoViewPosition[1] = glm::vec4(position + right * eyeSeparation / 2.0f, 1.0f);

    stereoViewMatrix[0]       = glm::lookAt(glm::vec3(stereoViewPosition[0]), glm::vec3(stereoViewPosition[0]) + forward, up);
    stereoViewMatrix[1]       = glm::lookAt(glm::vec3(stereoViewPosition[1]), glm::vec3(stereoViewPosition[1]) + forward, up);
    stereoProjectionMatrix[0] = glm::perspective(fieldOfView, aspectRatio / 2, nearPlane, farPlane);
    stereoProjectionMatrix[1] = glm::perspective(fieldOfView, aspectRatio / 2, nearPlane, farPlane);
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
        auto rotationDelta = Inputs::Get()->GetRotationDelta();   // x---yaw y---pitch
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

        right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        up    = glm::normalize(glm::cross(right, forward));

        position += velocity.z * forward;
        position += velocity.x * right;
        position += velocity.y * up;
    }
}


const glm::vec4 Camera::GetZBufferParams() const
{
    return glm::vec4(1.0f - farPlane / nearPlane, farPlane / nearPlane, 1.0f / farPlane - 1.0f / nearPlane, 1.0f / nearPlane);
}

const uint32_t Camera::GetPixelHeight() const
{
    return Devices::Get()->GetWindow()->GetSize().x;
}
const uint32_t Camera::GetPixelWidth() const
{
    return Devices::Get()->GetWindow()->GetSize().y;
}
const glm::vec4 Camera::GetPixelSize() const
{
    return glm::vec4(GetPixelWidth(), GetPixelHeight(), 1.0f / GetPixelWidth(), 1.0f / GetPixelHeight());
}

const uint32_t Camera::GetStereoPixelHeight() const
{
    return Devices::Get()->GetWindow()->GetStereoSize().y;
}
const uint32_t Camera::GetStereoPixelWidth() const
{
    return Devices::Get()->GetWindow()->GetStereoSize().x;
}
const glm::vec4 Camera::GetStereoPixelSize() const
{
    return glm::vec4(GetStereoPixelWidth(), GetStereoPixelHeight(), 1.0f / GetStereoPixelWidth(), 1.0f / GetStereoPixelHeight());
}
}   // namespace MapleLeaf