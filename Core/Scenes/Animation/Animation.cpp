#include "Animation.hpp"
#include "glm/fwd.hpp"

namespace MapleLeaf {
namespace {
const double kEpsilonTime = 1e-5f;

// Bezier form hermite spline
glm::vec3 interpolateHermite(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    glm::vec3 b0 = p1;
    glm::vec3 b1 = p1 + (p2 - p0) * 0.5f / 3.f;
    glm::vec3 b2 = p2 - (p3 - p1) * 0.5f / 3.f;
    glm::vec3 b3 = p2;

    glm::vec3 q0 = lerp(b0, b1, t);
    glm::vec3 q1 = lerp(b1, b2, t);
    glm::vec3 q2 = lerp(b2, b3, t);

    glm::vec3 qq0 = lerp(q0, q1, t);
    glm::vec3 qq1 = lerp(q1, q2, t);

    return lerp(qq0, qq1, t);
}

// Bezier hermite slerp
glm::quat interpolateHermite(const glm::quat& r0, const glm::quat& r1, const glm::quat& r2, const glm::quat& r3, float t)
{
    glm::quat b0 = r1;
    glm::quat b1 = r1 + (r2 - r0) * 0.5f / 3.0f;
    glm::quat b2 = r2 - (r3 - r1) * 0.5f / 3.0f;
    glm::quat b3 = r2;

    glm::quat q0 = slerp(b0, b1, t);
    glm::quat q1 = slerp(b1, b2, t);
    glm::quat q2 = slerp(b2, b3, t);

    glm::quat qq0 = slerp(q0, q1, t);
    glm::quat qq1 = slerp(q1, q2, t);

    return slerp(qq0, qq1, t);
}

// This function performs linear extrapolation when either t < 0 or t > 1
Animation::Keyframe interpolateLinear(const Animation::Keyframe& k0, const Animation::Keyframe& k1, float t)
{
    Animation::Keyframe result;
    result.translation = lerp(k0.translation, k1.translation, t);
    result.scaling     = lerp(k0.scaling, k1.scaling, t);
    result.rotation    = slerp(k0.rotation, k1.rotation, t);
    result.time        = glm::lerp(k0.time, k1.time, (double)t);
    return result;
}

Animation::Keyframe interpolateHermite(const Animation::Keyframe& k0, const Animation::Keyframe& k1, const Animation::Keyframe& k2,
                                       const Animation::Keyframe& k3, float t)
{
    assert(t >= 0.f && t <= 1.f);
    Animation::Keyframe result;
    result.translation = interpolateHermite(k0.translation, k1.translation, k2.translation, k3.translation, t);
    result.scaling     = lerp(k1.scaling, k2.scaling, t);
    result.rotation    = interpolateHermite(k0.rotation, k1.rotation, k2.rotation, k3.rotation, t);
    result.time        = glm::lerp(k1.time, k2.time, (double)t);
    return result;
}
}   // namespace

std::shared_ptr<Animation> Animation::create(const std::string& name, NodeID nodeID, double duration)
{
    return std::shared_ptr<Animation>(new Animation(name, nodeID, duration));
}

Animation::Animation(const std::string& name, NodeID nodeID, double duration)
    : mName(name)
    , mNodeID(nodeID)
    , mDuration(duration)
{}

glm::mat4 Animation::animate(double currentTime)
{
    double time = currentTime;
    if (time < mKeyframes.front().time || time > mKeyframes.back().time) {
        time = calcSampleTime(currentTime);
    }

    // Determine if the animation behaves linearly outside of defined keyframes.
    bool isLinearPostInfinity = time > mKeyframes.back().time && this->getPostInfinityBehavior() == Behavior::Linear;
    bool isLinearPreInfinity  = time < mKeyframes.front().time && this->getPreInfinityBehavior() == Behavior::Linear;

    Keyframe interpolated;

    if (isLinearPreInfinity && mKeyframes.size() > 1) {
        const auto& k0              = mKeyframes.front();
        auto        k1              = interpolate(mInterpolationMode, k0.time + kEpsilonTime);
        double      segmentDuration = k1.time - k0.time;
        float       t               = (float)((time - k0.time) / segmentDuration);
        interpolated                = interpolateLinear(k0, k1, t);
    }
    else if (isLinearPostInfinity && mKeyframes.size() > 1) {
        const auto& k1              = mKeyframes.back();
        auto        k0              = interpolate(mInterpolationMode, k1.time - kEpsilonTime);
        double      segmentDuration = k1.time - k0.time;
        float       t               = (float)((time - k0.time) / segmentDuration);
        interpolated                = interpolateLinear(k0, k1, t);
    }
    else {
        interpolated = interpolate(mInterpolationMode, time);
    }

    glm::mat4 translate = glm::translate(glm::mat4(1.0f), interpolated.translation);
    glm::mat4 rotate    = glm::mat4_cast(interpolated.rotation);
    glm::mat4 scale     = glm::scale(glm::mat4(1.0f), interpolated.scaling);
    glm::mat4 result    = translate * rotate * scale;

    return result;
}

Animation::Keyframe Animation::interpolate(InterpolationMode mode, double time) const
{
    assert(!mKeyframes.empty());

    // Validate cached frame index.
    size_t frameIndex = glm::clamp(mCachedFrameIndex, (size_t)0, mKeyframes.size() - 1);
    if (time < mKeyframes[frameIndex].time) frameIndex = 0;

    // Find frame index.
    while (frameIndex < mKeyframes.size() - 1) {
        if (mKeyframes[frameIndex + 1].time > time) break;
        frameIndex++;
    }

    // Cache frame index;
    mCachedFrameIndex = frameIndex;

    // Compute index of adjacent frame including optional warping.
    auto adjacentFrame = [this](size_t frame, int32_t offset = 1) {
        size_t count = mKeyframes.size();
        return mEnableWarping ? (frame + count + offset) % count : glm::clamp(frame + offset, (size_t)0, count - 1);
    };

    if (mode == InterpolationMode::Linear || mKeyframes.size() < 4) {
        const auto& k0              = mKeyframes[frameIndex];
        const auto& k1              = mKeyframes[adjacentFrame(frameIndex)];
        double      segmentDuration = k1.time - k0.time;
        if (mEnableWarping && segmentDuration < 0.0) segmentDuration += mDuration;
        float t = glm::clamp((segmentDuration > 0.0 ? (time - k0.time) / segmentDuration : 1.0), 0.0, 1.0);

        return interpolateLinear(k0, k1, t);
    }
    else if (mode == InterpolationMode::Hermite) {
        const auto& k0 = mKeyframes[adjacentFrame(frameIndex, -1)];
        const auto& k1 = mKeyframes[frameIndex];
        const auto& k2 = mKeyframes[adjacentFrame(frameIndex)];
        const auto& k3 = mKeyframes[adjacentFrame(frameIndex, 2)];

        double segmentDuration = k2.time - k1.time;
        if (mEnableWarping && segmentDuration < 0.0) segmentDuration += mDuration;
        float t = glm::clamp((segmentDuration > 0.0 ? (time - k1.time) / segmentDuration : 1.0), 0.0, 1.0);

        return interpolateHermite(k0, k1, k2, k3, t);
    }
    else {
        throw std::runtime_error("Unsupported interpolation mode");
    }
}

double Animation::calcSampleTime(double currentTime)
{
    double modifiedTime      = currentTime;
    double firstKeyframeTime = mKeyframes.front().time;
    double lastKeyframeTime  = mKeyframes.back().time;
    double duration          = mDuration;

    assert(currentTime < firstKeyframeTime || currentTime > lastKeyframeTime);

    Behavior behavior = currentTime < firstKeyframeTime ? mPreInfinityBehavior : mPostInfinityBehavior;
    switch (behavior) {
    case Behavior::Constant: modifiedTime = glm::clamp(currentTime, firstKeyframeTime, lastKeyframeTime); break;
    case Behavior::Cycle:
        // Calculate the relative time
        modifiedTime = firstKeyframeTime + std::fmod(currentTime - firstKeyframeTime, duration);
        if (modifiedTime < firstKeyframeTime) modifiedTime += duration;
        break;
    case Behavior::Linear: break;
    case Behavior::Oscillate:
        // Calculate the relative time
        double offset = std::fmod(currentTime - firstKeyframeTime, 2 * duration);
        if (offset < 0) offset += 2 * duration;
        if (offset > duration) offset = 2 * duration - offset;
        modifiedTime = firstKeyframeTime + offset;
    }

    return modifiedTime;
}

void Animation::addKeyframe(const Keyframe& keyframe)
{
    assert(keyframe.time <= mDuration);

    if (mKeyframes.size() == 0 || mKeyframes[0].time > keyframe.time) {
        mKeyframes.insert(mKeyframes.begin(), keyframe);
    }
    else if (mKeyframes.back().time < keyframe.time) {
        mKeyframes.push_back(keyframe);
    }
    else {
        for (size_t i = 0; i < mKeyframes.size(); i++) {
            auto& current = mKeyframes[i];
            // If we already have a key-frame at the same time, replace it
            if (current.time == keyframe.time) {
                current = keyframe;
                return;
            }

            // If this is not the last frame, Check if we are in between frames
            if (i < mKeyframes.size() - 1) {
                auto& Next = mKeyframes[i + 1];
                if (current.time < keyframe.time && Next.time > keyframe.time) {
                    mKeyframes.insert(mKeyframes.begin() + i + 1, keyframe);
                    return;
                }
            }
        }
        // If we got here, need to push it to the end of the list
        mKeyframes.push_back(keyframe);
    }
}

const Animation::Keyframe& Animation::getKeyframe(double time) const
{
    assert(doesKeyframeExists(time));
    for (const auto& keyframe : mKeyframes) {
        if (keyframe.time == time) return keyframe;
    }
    throw std::runtime_error("Keyframe not found");
}

bool Animation::doesKeyframeExists(double time) const
{
    for (const auto& keyframe : mKeyframes) {
        if (keyframe.time == time) return true;
    }
    return false;
}
}   // namespace MapleLeaf