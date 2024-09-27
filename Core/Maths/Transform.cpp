#include "Transform.hpp"
#include "AnimationController.hpp"
#include "Entity.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace MapleLeaf {
Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : position(position)
    , rotation(rotation)
    , scale(scale)
{
    prevWorldMatrix = GetWorldMatrix();
    updateStatus    = UpdateStatus::Transformation;
}

Transform::Transform(const glm::mat4 modelMatrix)
{
    glm::vec3 scale;
    glm::quat quaternion;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(modelMatrix, scale, quaternion, translation, skew, perspective);

    position = translation;

    rotation.x = atan2(2.0f * (quaternion.y * quaternion.z + quaternion.w * quaternion.x),
                       quaternion.w * quaternion.w - quaternion.x * quaternion.x - quaternion.y * quaternion.y + quaternion.z * quaternion.z);
    rotation.y = asin(-2.0f * (quaternion.x * quaternion.z - quaternion.w * quaternion.y));
    rotation.z = atan2(2.0f * (quaternion.x * quaternion.y + quaternion.w * quaternion.z),
                       quaternion.w * quaternion.w + quaternion.x * quaternion.x - quaternion.y * quaternion.y - quaternion.z * quaternion.z);
    // rotation    = glm::eulerAngles(quaternion);
    this->scale = scale;

    prevWorldMatrix = GetWorldMatrix();
    updateStatus    = UpdateStatus::Transformation;
}

Transform::~Transform()
{
    if (worldTransform) delete worldTransform;

    for (auto& child : children) child->parent = nullptr;

    if (parent) parent->RemoveChild(this);
}

void Transform::Update()
{
    prevWorldMatrix = GetWorldMatrix();
    if (const auto& ani = this->GetEntity()->GetComponent<AnimationController>(); ani != nullptr && ani->isMatrixChanged()) {
        glm::mat4 modelMatrix = ani->getLocalMatrix();

        glm::vec3 scale;
        glm::quat quaternion;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(modelMatrix, scale, quaternion, translation, skew, perspective);

        position = translation;

        rotation.x  = atan2(2.0f * (quaternion.y * quaternion.z + quaternion.w * quaternion.x),
                           quaternion.w * quaternion.w - quaternion.x * quaternion.x - quaternion.y * quaternion.y + quaternion.z * quaternion.z);
        rotation.y  = asin(-2.0f * (quaternion.x * quaternion.z - quaternion.w * quaternion.y));
        rotation.z  = atan2(2.0f * (quaternion.x * quaternion.y + quaternion.w * quaternion.z),
                           quaternion.w * quaternion.w + quaternion.x * quaternion.x - quaternion.y * quaternion.y - quaternion.z * quaternion.z);
        this->scale = scale;

        updateStatus = UpdateStatus::Transformation;
    }
    else {
        if (this->parent != nullptr && this->parent->GetUpdateStatus() == UpdateStatus::Transformation)
            updateStatus = UpdateStatus::Transformation;
        else
            updateStatus = UpdateStatus::None;
    }
    // updateStatus = UpdateStatus::None;
}

glm::mat4 Transform::GetWorldMatrix() const
{
    auto worldTransform = GetWorldTransform();

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), worldTransform->scale);

    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix           = glm::rotate(rotationMatrix, worldTransform->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    rotationMatrix           = glm::rotate(rotationMatrix, worldTransform->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix           = glm::rotate(rotationMatrix, worldTransform->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));


    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), worldTransform->position);

    return translationMatrix * rotationMatrix * scaleMatrix;
}

glm::vec3 Transform::GetPosition() const
{
    return GetWorldTransform()->position;
}

glm::vec3 Transform::GetRotation() const
{
    return GetWorldTransform()->rotation;
}

glm::vec3 Transform::GetScale() const
{
    return GetWorldTransform()->scale;
}

void Transform::SetParent(Transform* parent)
{
    if (this->parent) this->parent->RemoveChild(this);

    this->parent = parent;

    if (this->parent) this->parent->AddChild(this);
}

void Transform::SetParent(Entity* parent)
{
    SetParent(parent->GetComponent<Transform>());
}

bool Transform::operator==(const Transform& rhs) const
{
    return position == rhs.position && rotation == rhs.rotation && scale == rhs.scale;
}

bool Transform::operator!=(const Transform& rhs) const
{
    return !operator==(rhs);
}

Transform operator*(const Transform& lhs, const Transform& rhs)
{
    glm::vec4 newPosition = glm::vec4(rhs.position, 1.0f);
    // for (uint32_t row = 0; row < 4; row++) {
    //     newPosition[row] = lhs.GetWorldMatrix()[row][0] * rhs.position.x + lhs.GetWorldMatrix()[row][1] * rhs.position.y +
    //                        lhs.GetWorldMatrix()[row][2] * rhs.position.z + lhs.GetWorldMatrix()[row][3] * 1.0f;
    // }
    newPosition = lhs.GetWorldMatrix() * newPosition;
    return {glm::vec3(newPosition), lhs.rotation + rhs.rotation, lhs.scale * rhs.scale};
}

Transform& Transform::operator*=(const Transform& rhs)
{
    return *this = *this * rhs;
}

std::ostream& operator<<(std::ostream& stream, const Transform& transform)
{
    return stream << transform.position << ", " << transform.rotation << ", " << transform.scale;
}

const Transform* Transform::GetWorldTransform() const
{
    if (!parent) {
        if (worldTransform) {
            delete worldTransform;
            worldTransform = nullptr;
        }

        return this;
    }

    if (!worldTransform) {
        worldTransform = new Transform();
    }

    *worldTransform = *parent->GetWorldTransform() * *this;
    return worldTransform;
}

void Transform::AddChild(Transform* child)
{
    children.emplace_back(child);
}

void Transform::RemoveChild(Transform* child)
{
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}
}   // namespace MapleLeaf