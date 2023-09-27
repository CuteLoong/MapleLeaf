#pragma once

#include "Component.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <string>

namespace MapleLeaf {
class Camera: public Component::Registrar<Camera>
{
public:
    Camera()
        : nearPlane(0.1f)
        , farPlane(100.0f)
        , fieldOfView(glm::radians(60.0f))
        , up(glm::vec3(0.0f, 1.0f, 0.0f))
        , forward(glm::vec3(0.0f, 0.0f, -1.0f))   // rotation(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f))
        , position(glm::vec3(0.0f, 0.0f, 2.0f))
    {}

    ~Camera() = default;

    void Start();
    void Update();

    void UpdateByTransform();
    void UpdateByInput();

    /**
     * Gets the distance of the near pane of the view frustum.
     * @return The distance of the near pane of the view frustum.
     */
    float GetNearPlane() const { return nearPlane; }
    void  SetNearPlane(float nearPlane) { this->nearPlane = nearPlane; }

    /**
     * Gets the distance of the view frustum's far plane.
     * @return The distance of the view frustum's far plane.
     */
    float GetFarPlane() const { return farPlane; }
    void  SetFarPlane(float farPlane) { this->farPlane = farPlane; }

    /**
     * Gets the field of view angle for the view frustum.
     * @return The field of view angle for the view frustum.
     */
    float GetFieldOfView() const { return fieldOfView; }
    void  SetFieldOfView(float fieldOfView) { this->fieldOfView = fieldOfView; }

    float GetAspectRatio() const { return aspectRatio; }
    void  SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }

    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetRotation() const { return rotation; }
    const glm::vec3& GetVelocity() const { return velocity; }
    const glm::vec3& GetUpVector() const { return up; }
    const glm::vec3& GetForward() const { return forward; }
    void             SetUpVector(const glm::vec3& up) { this->up = up; }
    void             SetForward(const glm::vec3& forward) { this->forward = forward; }
    void             SetPosition(const glm::vec3& position) { this->position = position; }
    void             SetRotation(const glm::vec3& rotation) { this->rotation = rotation; }
    void             SetVelocity(const glm::vec3& velocity) { this->velocity = velocity; }

    const std::string& GetName() const { return name; }
    void               SetName(const std::string& name) { this->name = name; }

    const glm::mat4& GetViewMatrix() const { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }

    const glm::mat4& GetInverseViewMatrix() const { return invViewMatrix; }
    const glm::mat4& GetInverseProjectionMatrix() const { return invProjectionMatrix; }

protected:
    std::string name;

    float nearPlane, farPlane;
    float fieldOfView, aspectRatio;

    glm::vec3 position;
    glm::vec3 rotation;   // (0, 0, -1.0) ROTATION
    glm::vec3 velocity;

    glm::vec3 up;
    glm::vec3 forward;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 invViewMatrix;
    glm::mat4 invProjectionMatrix;
};
}   // namespace MapleLeaf