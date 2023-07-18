#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace MapleLeaf {
class Camera
{
public:
    Camera()
        : nearPlane(0.1f)
        , farPlane(1000.0f)
        , fieldOfView(glm::radians(60.0f))
    {}

    virtual ~Camera() = default;

    virtual void Start() {}
    virtual void Update() {}

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

    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetRotation() const { return rotation; }
    const glm::vec3& GetVelocity() const { return velocity; }

    const glm::mat4& GetViewMatrix() const { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }

protected:
    float nearPlane, farPlane;
    float fieldOfView;

    glm::vec3 position;
    glm::vec3 rotation; // x----Pitch y----Yaw
    glm::vec3 velocity;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};
}   // namespace MapleLeaf