#pragma once

#include "Camera.hpp"
#include "EntityHolder.hpp"
#include "SystemHolder.hpp"

namespace MapleLeaf {
class Scene
{
    friend class Scenes;

public:
    explicit Scene(std::unique_ptr<Camera>&& camera);
    virtual ~Scene() = default;

    virtual void Start() = 0;
    virtual void Update();

    template<typename T>
    bool HasSystem() const
    {
        return systems.Has<T>();
    }

    template<typename T>
    T* GetSystem() const
    {
        return systems.Get<T>();
    }

    template<typename T, typename... Args>
    void AddSystem(Args&&... args)
    {
        systems.Add<T>(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template<typename T>
    void RemoveSystem()
    {
        systems.Remove<T>();
    }

    void ClearSystems() { systems.Clear(); }

    Entity* GetEntity(const std::string& name) const { return entities.GetEntity(name); }

    Entity* CreateEntity() { return entities.CreateEntity(); }

    std::vector<Entity*> QueryAllEntities() { return entities.QueryAll(); }

    template<typename T>
    T* GetComponent(bool allowDisabled = false)
    {
        return entities.GetComponent<T>(allowDisabled);
    }

    template<typename T>
    std::vector<T*> GetComponents(bool allowDisabled = false)
    {
        return entities.GetComponents<T>(allowDisabled);
    }

    void ClearEntities() { entities.Clear(); }

    Camera* GetCamera() const { return camera.get(); }
    void    SetCamera(Camera* camera) { this->camera.reset(camera); }

    virtual bool IsPaused() const = 0;

private:
    bool                    started = false;
    SystemHolder            systems;
    EntityHolder            entities;
    std::unique_ptr<Camera> camera;
};
}   // namespace MapleLeaf