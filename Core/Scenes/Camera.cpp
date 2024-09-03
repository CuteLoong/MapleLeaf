#include "Camera.hpp"
#include "Engine.hpp"
#include "Inputs.hpp"
#include "Scenes.hpp"
#include <limits>

#include "config.h"

namespace MapleLeaf {
Camera::Camera()
    : nearPlane(0.1f)
    , farPlane(100.0f)
    , fieldOfView(glm::radians(60.0f))
    , eyeSeparation(0.5f)                     // 4.0 Pica,
    , up(glm::vec3(0.0f, 1.0f, 0.0f))
    , forward(glm::vec3(0.0f, 0.0f, -1.0f))   // rotation(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f))
    , position(glm::vec3(0.0f, 0.0f, 2.0f))
{
    right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    frameID = 0;
}

void Camera::Start()
{
    UpdateByTransform();
}
void Camera::Update()
{
    frameID = (frameID + 1) % std::numeric_limits<uint32_t>::max();

    const auto& transform = GetEntity()->GetComponent<Transform>();
    if (transform->GetUpdateStatus() == Transform::UpdateStatus::Transformation) {
        UpdateByTransform();
    }
    else {
        // UpdateByInput();
        // UpdateByCPU();
    }

    UpdateCameraInfo();
    UpdateStereoCameraInfo();

    // if (frameID != 0 && frameID < 16440 && frameID % 120 == 0) {
    //     Graphics::Get()->CaptureScreenshot("Screenshots/" + std::to_string(frameID / 120) + ".png");
    // }
    // if (frameID != 0 && frameID < 884 && frameID % 4 == 0) Graphics::Get()->CaptureScreenshot("Screenshots/" + std::to_string(frameID / 4) +
    // ".png"); if (frameID == 120) {
    //     Graphics::Get()->CaptureScreenshot("Screenshots/" + std::to_string(frameID / 120) + ".png");
    // }
}

void Camera::UpdateByCPU()
{
    // if (frameID != 0 && frameID < 14400 && frameID % 120 == 0) position = position + 0.1f * forward;
    // if (frameID != 0 && frameID < 14520 && frameID % 120 == 0) {
    //     position = position - 0.2f * right;
    // } // Room
    if (frameID != 0 && frameID < 14520 && frameID % 120 == 0) {
        rotation.y += glm::radians(5.0f / 60.0f);
        rotation.x += 0.0f;
        rotation.x = std::clamp(rotation.x, glm::radians(-90.0f), glm::radians(90.0f));

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix           = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        forward                  = rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

        forward = glm::normalize(forward);
        right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        up      = glm::normalize(glm::cross(right, forward));

        position = position + 0.5f * forward;
    }


    // if (frameID != 0 && frameID <= 7260 && frameID % 120 == 0) {
    // rotation.y += glm::radians(2.0f / 60.0f);
    // rotation.x += 0.0f;
    // rotation.x = std::clamp(rotation.x, glm::radians(-90.0f), glm::radians(90.0f));

    // glm::mat4 rotationMatrix = glm::mat4(1.0f);
    // rotationMatrix           = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    // rotationMatrix           = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    // rotationMatrix           = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    // forward                  = rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    // forward = glm::normalize(forward);
    // right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    // up      = glm::normalize(glm::cross(right, forward));

    // position = position + 0.3f * forward;
    // }
    // else if (frameID > 7260 && frameID < 14520 && frameID % 120 == 0)
    //     position = position - 0.2f * right;
}

void Camera::UpdateByTransform()
{
    const auto& transform = GetEntity()->GetComponent<Transform>();
    up                    = transform->GetWorldMatrix()[1];
    forward               = -transform->GetWorldMatrix()[2];
    position              = transform->GetWorldMatrix()[3];
    rotation              = transform->GetRotation();

    right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    up    = glm::normalize(glm::cross(right, forward));
}

void Camera::UpdateByInput()
{
    auto delta = Engine::Get()->GetDelta().AsSeconds();

    if (!Scenes::Get()->GetScene()->IsPaused()) {
        auto positionDelta = Inputs::Get()->GetPositionDelta();
        auto rotationDelta = Inputs::Get()->GetRotationDelta();   // x---yaw y---pitch

        if (positionDelta == glm::vec3(0.0f) && rotationDelta == glm::vec2(0.0f)) return;

        Inputs::Get()->ResetPositionDelta();
        Inputs::Get()->ResetRotationDelta();

        velocity = 5.0f * delta * positionDelta;

        float mouseSensity = 0.0001f * Engine::Get()->GetUps();

        rotation.y += rotationDelta.x * mouseSensity;
        rotation.x += rotationDelta.y * mouseSensity;
        // rotation.x = std::clamp(rotation.x, glm::radians(-90.0f), glm::radians(90.0f));

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

        frameID = 0;
    }
}

void Camera::UpdateCameraInfo()
{
    prevProjectionMatrix = projectionMatrix;
    prevViewMatrix       = viewMatrix;

    viewMatrix       = glm::lookAt(position, position + forward, up);
    projectionMatrix = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);

    invViewMatrix       = glm::inverse(viewMatrix);
    invProjectionMatrix = glm::inverse(projectionMatrix);

    glm::vec3 farPlaneCenter       = position + forward * farPlane;
    glm::vec3 nearPlaneCenter      = position + forward * nearPlane;
    float     farPlaneHalfVertical = farPlane * std::tan(fieldOfView / 2.0f);
    float     farPlaneHalfHorizon  = farPlaneHalfVertical * aspectRatio;

    glm::vec3 leftTop     = farPlaneCenter - farPlaneHalfHorizon * right + farPlaneHalfVertical * up;
    glm::vec3 rightTop    = farPlaneCenter + farPlaneHalfHorizon * right + farPlaneHalfVertical * up;
    glm::vec3 leftBottom  = farPlaneCenter - farPlaneHalfHorizon * right - farPlaneHalfVertical * up;
    glm::vec3 rightBottom = farPlaneCenter + farPlaneHalfHorizon * right - farPlaneHalfVertical * up;

    frustumVector[0] = glm::normalize(glm::vec4(leftTop - position, 0.0f));
    frustumVector[1] = glm::normalize(glm::vec4(rightTop - position, 0.0f));
    frustumVector[2] = glm::normalize(glm::vec4(leftBottom - position, 0.0f));
    frustumVector[3] = glm::normalize(glm::vec4(rightBottom - position, 0.0f));

    frustumPlane[0] = glm::vec4(glm::normalize(glm::cross(glm::vec3(frustumVector[2]), glm::vec3(frustumVector[0]))), 0.0f);
    frustumPlane[1] = glm::vec4(glm::normalize(glm::cross(glm::vec3(frustumVector[1]), glm::vec3(frustumVector[3]))), 0.0f);
    frustumPlane[2] = glm::vec4(glm::normalize(glm::cross(glm::vec3(frustumVector[3]), glm::vec3(frustumVector[2]))), 0.0f);
    frustumPlane[3] = glm::vec4(glm::normalize(glm::cross(glm::vec3(frustumVector[0]), glm::vec3(frustumVector[1]))), 0.0f);
    frustumPlane[4] = glm::vec4(forward, 0.0f);
    frustumPlane[5] = glm::vec4(-forward, 0.0f);

    frustumPlane[0].w = -glm::dot(glm::vec3(frustumPlane[0]), position);
    frustumPlane[1].w = -glm::dot(glm::vec3(frustumPlane[1]), position);
    frustumPlane[2].w = -glm::dot(glm::vec3(frustumPlane[2]), position);
    frustumPlane[3].w = -glm::dot(glm::vec3(frustumPlane[3]), position);
    frustumPlane[4].w = -glm::dot(glm::vec3(frustumPlane[4]), nearPlaneCenter);
    frustumPlane[5].w = -glm::dot(glm::vec3(frustumPlane[5]), farPlaneCenter);
}

void Camera::UpdateStereoCameraInfo()
{
    prevStereoViewMatrix[0] = stereoViewMatrix[0];
    prevStereoViewMatrix[1] = stereoViewMatrix[1];

    prevStereoProjectionMatrix[0] = stereoProjectionMatrix[0];
    prevStereoProjectionMatrix[1] = stereoProjectionMatrix[1];

    stereoViewPosition[0] = glm::vec4(position - right * eyeSeparation / 2.0f, 1.0f);
    stereoViewPosition[1] = glm::vec4(position + right * eyeSeparation / 2.0f, 1.0f);

    stereoViewMatrix[0]       = glm::lookAt(glm::vec3(stereoViewPosition[0]), glm::vec3(stereoViewPosition[0]) + forward, up);
    stereoViewMatrix[1]       = glm::lookAt(glm::vec3(stereoViewPosition[1]), glm::vec3(stereoViewPosition[1]) + forward, up);
    stereoProjectionMatrix[0] = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
    stereoProjectionMatrix[1] = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);

    invStereoViewMatrix[0]       = glm::inverse(stereoViewMatrix[0]);
    invStereoViewMatrix[1]       = glm::inverse(stereoViewMatrix[1]);
    invStereoProjectionMatrix[0] = glm::inverse(stereoProjectionMatrix[0]);
    invStereoProjectionMatrix[1] = glm::inverse(stereoProjectionMatrix[1]);

    float farPlaneHalfVertical = farPlane * std::tan(fieldOfView / 2.0f);
    float farPlaneHalfHorizon  = farPlaneHalfVertical * aspectRatio;

    for (int i = 0; i <= 1; i++) {
        const glm::vec3& curEyePosition  = glm::vec3(stereoViewPosition[i]);
        glm::vec3        farPlaneCenter  = curEyePosition + forward * farPlane;
        glm::vec3        nearPlaneCenter = curEyePosition + forward * nearPlane;

        glm::vec3 leftTop     = farPlaneCenter - farPlaneHalfHorizon * right + farPlaneHalfVertical * up;
        glm::vec3 rightTop    = farPlaneCenter + farPlaneHalfHorizon * right + farPlaneHalfVertical * up;
        glm::vec3 leftBottom  = farPlaneCenter - farPlaneHalfHorizon * right - farPlaneHalfVertical * up;
        glm::vec3 rightBottom = farPlaneCenter + farPlaneHalfHorizon * right - farPlaneHalfVertical * up;

        stereoFrustumVector[i][0] = glm::normalize(glm::vec4(leftTop - curEyePosition, 0.0f));
        stereoFrustumVector[i][1] = glm::normalize(glm::vec4(rightTop - curEyePosition, 0.0f));
        stereoFrustumVector[i][2] = glm::normalize(glm::vec4(leftBottom - curEyePosition, 0.0f));
        stereoFrustumVector[i][3] = glm::normalize(glm::vec4(rightBottom - curEyePosition, 0.0f));

        stereoFrustumPlane[i][0] =
            glm::vec4(glm::normalize(glm::cross(glm::vec3(stereoFrustumVector[i][2]), glm::vec3(stereoFrustumVector[i][0]))), 0.0f);
        stereoFrustumPlane[i][1] =
            glm::vec4(glm::normalize(glm::cross(glm::vec3(stereoFrustumVector[i][1]), glm::vec3(stereoFrustumVector[i][3]))), 0.0f);
        stereoFrustumPlane[i][2] =
            glm::vec4(glm::normalize(glm::cross(glm::vec3(stereoFrustumVector[i][3]), glm::vec3(stereoFrustumVector[i][2]))), 0.0f);
        stereoFrustumPlane[i][3] =
            glm::vec4(glm::normalize(glm::cross(glm::vec3(stereoFrustumVector[i][0]), glm::vec3(stereoFrustumVector[i][1]))), 0.0f);
        stereoFrustumPlane[i][4] = glm::vec4(forward, 0.0f);
        stereoFrustumPlane[i][5] = glm::vec4(-forward, 0.0f);

        stereoFrustumPlane[i][0].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][0]), curEyePosition);
        stereoFrustumPlane[i][1].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][1]), curEyePosition);
        stereoFrustumPlane[i][2].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][2]), curEyePosition);
        stereoFrustumPlane[i][3].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][3]), curEyePosition);
        stereoFrustumPlane[i][4].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][4]), nearPlaneCenter);
        stereoFrustumPlane[i][5].w = -glm::dot(glm::vec3(stereoFrustumPlane[i][5]), farPlaneCenter);
    }
}

const glm::vec4 Camera::GetZBufferParams() const
{
    return glm::vec4(1.0f - farPlane / nearPlane, farPlane / nearPlane, 1.0f / farPlane - 1.0f / nearPlane, 1.0f / nearPlane);
}

const glm::vec4 Camera::GetProjectionParams() const
{
    return glm::vec4(-1.0f, nearPlane, farPlane, 1.0f / farPlane);
}

const uint32_t Camera::GetPixelHeight() const
{
    return Devices::Get()->GetWindow()->GetSize().y;
}
const uint32_t Camera::GetPixelWidth() const
{
    return Devices::Get()->GetWindow()->GetSize().x;
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

void Camera::PushUniforms(UniformHandler& uniformObject)
{
    uniformObject.Push("projection", projectionMatrix);
    uniformObject.Push("prevProjection", prevProjectionMatrix);
    uniformObject.Push("view", viewMatrix);
    uniformObject.Push("prevView", prevViewMatrix);
    uniformObject.Push("invProjection", invProjectionMatrix);
    uniformObject.Push("invView", invViewMatrix);
    uniformObject.Push("stereoProjection", stereoProjectionMatrix);
    uniformObject.Push("prevStereoProjection", prevStereoProjectionMatrix);
    uniformObject.Push("stereoView", stereoViewMatrix);
    uniformObject.Push("prevStereoView", prevStereoViewMatrix);
    uniformObject.Push("invStereoProjection", invStereoProjectionMatrix);
    uniformObject.Push("invStereoView", invStereoViewMatrix);
    uniformObject.Push("frustumVector", frustumVector);
    uniformObject.Push("frustumPlane", frustumPlane);
    uniformObject.Push("stereoLeftFrustumVector", stereoFrustumVector[0]);
    uniformObject.Push("stereoRightFrustumVector", stereoFrustumVector[1]);
    uniformObject.Push("stereoLeftFrustumPlane", stereoFrustumPlane[0]);
    uniformObject.Push("stereoRightFrustumPlane", stereoFrustumPlane[1]);
    uniformObject.Push("zBufferParams", GetZBufferParams());
    uniformObject.Push("projectionParams", GetProjectionParams());
    uniformObject.Push("pixelSize", GetPixelSize());
    uniformObject.Push("stereoPixelSize", GetStereoPixelSize());
    uniformObject.Push("cameraPosition", glm::vec4(position, 1.0f));
    uniformObject.Push("cameraStereoPosition", stereoViewPosition);
    uniformObject.Push("frameID", frameID);
}
}   // namespace MapleLeaf