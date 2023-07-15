#include "Transform.hpp"
#include "Entity.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace MapleLeaf {
Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : position(position)
    , rotation(rotation)
    , scale(scale)
{}

Transform::Transform(const glm::mat4 modelMatrix)
{
    glm::vec3 scale;
    glm::quat quaternion;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(modelMatrix, scale, quaternion, translation, skew, perspective);

    position    = translation;
    rotation    = glm::eulerAngles(quaternion);
    this->scale = scale;
}

Transform::~Transform()
{
    if(worldTransform) delete worldTransform;

    for (auto& child : children) child->parent = nullptr;

    if (parent) parent->RemoveChild(this);
}

glm::mat4 Transform::GetWorldMatrix() const
{
    auto worldTransform = GetWorldTransform();

    glm::mat4 trans = glm::mat4(1.0f);
    trans           = glm::scale(trans, rotation);
    trans           = glm::rotate(trans, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    trans           = glm::rotate(trans, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    trans           = glm::rotate(trans, rotation.y, glm::vec3(0.0f, 0.0f, 1.0f));
    trans           = glm::translate(trans, position);

    return trans;
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
    for (uint32_t row = 0; row < 4; row++) {
        newPosition[row] = lhs.GetWorldMatrix()[row][0] * rhs.position.x + lhs.GetWorldMatrix()[row][1] * rhs.position.y +
                           lhs.GetWorldMatrix()[row][2] * rhs.position.z + lhs.GetWorldMatrix()[row][3] * 1.0f;
    }
    return {glm::vec3(newPosition), lhs.rotation + rhs.rotation, lhs.scale + rhs.scale};
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