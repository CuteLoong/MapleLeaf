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

private:
    std::unique_ptr<Scene> scene;
};
}   // namespace MapleLeaf