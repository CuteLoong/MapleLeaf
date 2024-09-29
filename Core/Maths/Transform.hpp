#pragma once

#include "Component.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
class Transform : public Component::Registrar<Transform>
{
    inline static const bool Registered = Register("transform");

public:
    enum class UpdateStatus
    {
        None,
        Transformation
    };

    Transform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f));
    Transform(const glm::mat4 modelMatrix);
    ~Transform();

    void Update() override;

    glm::mat4 GetWorldMatrix() const;
    glm::mat4 GetPrevWorldMatrix() const;

    glm::vec3 GetPosition() const;
    glm::vec3 GetRotation() const;
    glm::vec3 GetScale() const;
    glm::vec3 GetPrevPosition() const;
    glm::vec3 GetPrevRotation() const;
    glm::vec3 GetPrevScale() const;

    const glm::vec3& GetLocalPosition() const { return position; }
    void             SetLocalPosition(const glm::vec3& localPosition) { position = localPosition; }

    const glm::vec3& GetLocalRotation() const { return rotation; }
    void             SetLocalRotation(const glm::vec3& localRotation) { rotation = localRotation; }

    const glm::vec3& GetLocalScale() const { return scale; }
    void             SetLocalScale(const glm::vec3& localScale) { scale = localScale; }


    Transform* GetParent() const { return parent; }
    void       SetParent(Transform* parent);
    void       SetParent(Entity* parent);

    const Transform* GetWorldTransform() const;

    const std::vector<Transform*>& GetChildren() const { return children; }

    UpdateStatus GetUpdateStatus() const { return updateStatus; }

    bool operator==(const Transform& rhs) const;
    bool operator!=(const Transform& rhs) const;

    friend Transform operator*(const Transform& lhs, const Transform& rhs);

    Transform& operator*=(const Transform& rhs);

    friend std::ostream& operator<<(std::ostream& stream, const Transform& transform);

private:
    Transform(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale, const glm::vec3 prevPosition, const glm::vec3 prevRotation,
              const glm::vec3 prevScale);

    void AddChild(Transform* child);
    void RemoveChild(Transform* child);

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::vec3 prevPosition;
    glm::vec3 prevRotation;
    glm::vec3 prevScale;

    Transform*              parent = nullptr;
    std::vector<Transform*> children;
    mutable Transform*      worldTransform = nullptr;

    UpdateStatus updateStatus;
};
}   // namespace MapleLeaf