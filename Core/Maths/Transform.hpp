#pragma once

#include "Component.hpp"
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
    glm::vec3 GetPosition() const;
    glm::vec3 GetRotation() const;
    glm::vec3 GetScale() const;

    const glm::vec3& GetLocalPosition() const { return position; }
    void             SetLocalPosition(const glm::vec3& localPosition) { position = localPosition; }

    const glm::vec3& GetLocalRotation() const { return rotation; }
    void             SetLocalRotation(const glm::vec3& localRotation) { rotation = localRotation; }

    const glm::vec3& GetLocalScale() const { return scale; }
    void             SetLocalScale(const glm::vec3& localScale) { scale = localScale; }

    Transform* GetParent() const { return parent; }
    void       SetParent(Transform* parent);
    void       SetParent(Entity* parent);

    const std::vector<Transform*>& GetChildren() const { return children; }

    UpdateStatus GetUpdateStatus() const { return updateStatus; }

    bool operator==(const Transform& rhs) const;
    bool operator!=(const Transform& rhs) const;

    friend Transform operator*(const Transform& lhs, const Transform& rhs);

    Transform& operator*=(const Transform& rhs);

    friend std::ostream& operator<<(std::ostream& stream, const Transform& transform);

private:
    const Transform* GetWorldTransform() const;

    void AddChild(Transform* child);
    void RemoveChild(Transform* child);

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Transform*              parent = nullptr;
    std::vector<Transform*> children;
    mutable Transform*      worldTransform = nullptr;

    UpdateStatus updateStatus;
};
}   // namespace MapleLeaf