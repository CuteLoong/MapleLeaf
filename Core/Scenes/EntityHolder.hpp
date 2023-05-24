#pragma once

#include "Entity.hpp"

namespace MapleLeaf {
class EntityHolder : NonCopyable
{
public:
    EntityHolder();

    void Update();

    /**
     * Gets a Entity by name.
     * @param name The Entity name.
     * @return The entity.
     */
    Entity* GetEntity(const std::string& name) const;

    /**
     * Creates a new entity.
     * @return The Entity.
     */
    Entity* CreateEntity();

    /**
     * Adds a new entity to the spatial structure.
     * @param entity The entity to add.
     */
    void Add(std::unique_ptr<Entity>&& entity);

    /**
     * Removes an entity from the spatial structure.
     * @param entity The entity to remove.
     */
    void Remove(Entity* entity);

    /**
     * Moves an entity to another spatial structure.
     * @param entity The entity to move.
     * @param structure The structure to move to.
     */
    void Move(Entity* entity, EntityHolder& structure);

    /**
     * Removes all Entities.
     */
    void Clear();

    /**
     * Gets the size of this structure.
     * @return The structures size.
     */
    uint32_t GetSize() const { return static_cast<uint32_t>(entities.size()); }

    /**
     * Gets a set of all objects in the spatial structure.
     * @return The list specified by of all objects.
     */
    std::vector<Entity*> QueryAll();

    /**
     * Gets the first component of a type found in the spatial structure.
     * @tparam T The component type to get.
     * @param allowDisabled If disabled components will be included in this query.
     * @return The first component of the type found.
     */
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Component*>>>
    T* GetComponent(bool allowDisabled = false)
    {
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            auto component = (*it)->GetComponent<T>();

            if (component && (component->IsEnabled() || allowDisabled)) {
                return component;
            }
        }

        return nullptr;
    }

    /**
     * Returns a set of all components of a type in the spatial structure.
     * @tparam T The components type to get.
     * @param allowDisabled If disabled components will be included in this query.
     * @return The list specified by of all components that match the type.
     */
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Component*>>>
    std::vector<T*> GetComponents(bool allowDisabled = false)
    {
        std::vector<T*> components;

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            for (const auto& component : (*it)->GetComponents<T>()) {
                if (component && (component->IsEnabled() || allowDisabled)) {
                    components.emplace_back(component);
                }
            }
        }

        return components;
    }

    /**
     * If the structure contains the object.
     * @param object The object to check for.
     * @return If the structure contains the object.
     */
    bool Contains(Entity* object);

private:
    std::vector<std::unique_ptr<Entity>> entities;
};
}   // namespace MapleLeaf