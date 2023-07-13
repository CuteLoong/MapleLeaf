#pragma once

#include "Camera.hpp"
#include "Graphics.hpp"
#include "Scene.hpp"
#include "SceneGraph.hpp"

namespace MapleLeaf {
class Scenes : public Module::Registrar<Scenes>
{
    inline static const bool Registered = Register(Stage::Normal, Requires<Graphics>());

public:
    Scenes();

    void Update() override;

    Scene* GetScene() const { return scene.get(); }
    void   SetScene(std::unique_ptr<Scene>&& scene) { this->scene = std::move(scene); }

    // SceneNode* GetSceneRoot() const { return sceneRoot.get(); }
    // void       SetSceneRoot(SceneNode* sceneRoot) { this->sceneRoot.reset(sceneRoot); }

private:
    std::unique_ptr<Scene>     scene;
    // std::shared_ptr<SceneNode> sceneRoot;
};
}   // namespace MapleLeaf