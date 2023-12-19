#pragma once

#include "ASScene.hpp"
#include "Camera.hpp"
#include "EntityHolder.hpp"
#include "GPUScene.hpp"
#include "SystemHolder.hpp"

namespace MapleLeaf {
class Scene
{
    friend class Scenes;

public:
    Scene();
    virtual ~Scene()
    {
        gpuScene = nullptr;
        entities.Clear();
    }

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

    Camera* GetCamera() const { return camera; }
    void    SetCamera(Camera* camera) { this->camera = camera; }

    virtual bool IsPaused() const = 0;

    const glm::vec3& GetMinExtents() const { return minExtents; }
    const glm::vec3& GetMaxExtents() const { return maxExtents; }
    void             SetExtents(const glm::vec3& maxExtent, const glm::vec3& minExtent, const glm::mat4& transfrom);

    GPUScene* GetGpuScene() { return gpuScene.get(); }
    ASScene*  GetAsScene() { return asScene.get(); }

private:
    bool         started = false;
    SystemHolder systems;
    EntityHolder entities;
    Camera*      camera;

    std::unique_ptr<GPUScene> gpuScene;
    std::unique_ptr<ASScene>  asScene;


    glm::vec3 minExtents = glm::vec3(std::numeric_limits<float>::infinity());
    glm::vec3 maxExtents = glm::vec3(-std::numeric_limits<float>::infinity());
};
}   // namespace MapleLeaf