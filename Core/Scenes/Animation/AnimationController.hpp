#pragma once

#include "Animation.hpp"
#include <memory>

namespace MapleLeaf {
class AnimationController : public Component::Registrar<AnimationController>
{
    inline static const bool Registered = Register("AnimationController");

public:
    static const uint32_t kInvalidBoneID = -1;
    explicit AnimationController(const std::shared_ptr<Animation> animation = nullptr);
    ~AnimationController() = default;

    void Update() override;

    bool hasAnimations() const { return animation != nullptr; }

    std::shared_ptr<Animation> getAnimation() const { return animation; }

    void             setEnabled(bool enabled);
    bool             isEnabled() const { return mEnabled; };
    void             setIsLooped(bool looped);
    bool             isLooped() { return mLoopAnimations; }
    bool             isMatrixChanged() const { return matrixChanged; }
    const glm::mat4& getLocalMatrix() const { return localMatrix; }

private:
    std::shared_ptr<Animation> animation;
    glm::mat4                  localMatrix;
    bool                       matrixChanged;

    bool   mFirstUpdate = true;    ///< True if this is the first update.
    bool   mEnabled     = true;    ///< True if animations are enabled.
    bool   mPrevEnabled = false;   ///< True if animations were enabled in previous frame.
    double mTime        = 0.0;     ///< Global time of current frame.
    double mPrevTime    = 0.0;     ///< Global time of previous frame.

    int tmpCount = 0;

    bool   mLoopAnimations        = true;
    double mGlobalAnimationLength = 0;
};
}   // namespace MapleLeaf