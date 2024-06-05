#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "SceneGraph.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"
#include <memory>

namespace MapleLeaf {
class Animation
{
public:
    enum class InterpolationMode
    {
        Linear,
        Hermite,
    };

    enum class Behavior
    {
        Constant,
        Linear,
        Cycle,
        Oscillate,
    };

    struct Keyframe
    {
        double    time        = 0;
        glm::vec3 translation = glm::vec3(0, 0, 0);
        glm::vec3 scaling     = glm::vec3(1, 1, 1);
        glm::quat rotation    = glm::quat(1, 0, 0, 0);
    };

    static std::shared_ptr<Animation> create(const std::string& name, NodeID nodeID, double duration);

    const std::string& getName() const { return mName; }
    NodeID             getNodeID() const { return mNodeID; }
    void               setNodeID(NodeID id) { mNodeID = id; }
    double             getDuration() const { return mDuration; }
    Behavior           getPreInfinityBehavior() const { return mPreInfinityBehavior; }
    void               setPreInfinityBehavior(Behavior behavior) { mPreInfinityBehavior = behavior; }
    Behavior           getPostInfinityBehavior() const { return mPostInfinityBehavior; }
    void               setPostInfinityBehavior(Behavior behavior) { mPostInfinityBehavior = behavior; }
    InterpolationMode  getInterpolationMode() const { return mInterpolationMode; }
    void               setInterpolationMode(InterpolationMode interpolationMode) { mInterpolationMode = interpolationMode; }
    bool               isWarpingEnabled() const { return mEnableWarping; }
    void               setEnableWarping(bool enableWarping) { mEnableWarping = enableWarping; }
    void               addKeyframe(const Keyframe& keyframe);
    const Keyframe&    getKeyframe(double time) const;
    bool               doesKeyframeExists(double time) const;

    glm::mat4 animate(double currentTime);

private:
    Animation(const std::string& name, NodeID nodeID, double duration);

    Keyframe interpolate(InterpolationMode mode, double time) const;
    double   calcSampleTime(double currentTime);

    std::string mName;
    NodeID      mNodeID;
    double      mDuration;                                 // Includes any time before the first keyframe. May be Assimp or FBX specific.

    Behavior mPreInfinityBehavior  = Behavior::Constant;   // How the animation behaves before the first keyframe
    Behavior mPostInfinityBehavior = Behavior::Constant;   // How the animation behaves after the last keyframe

    InterpolationMode mInterpolationMode = InterpolationMode::Linear;
    bool              mEnableWarping     = false;

    std::vector<Keyframe> mKeyframes;
    mutable size_t        mCachedFrameIndex = 0;
};
}   // namespace MapleLeaf