#include "AniEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void AniEngine::Update(GameObject& obj, float dt) {
    if (!obj.isAnimating || obj.currentAnimationIndex < 0 || obj.currentAnimationIndex >= (int)obj.animations.size()) {
        return;
    }

    obj.animationTime += dt;
    const AnimationClip& clip = obj.animations[obj.currentAnimationIndex];
    
    if (obj.animationTime > clip.duration) {
        obj.animationTime = fmod(obj.animationTime, clip.duration);
    }

    CalculateBoneMatrices(obj, obj.animationTime);
}

void AniEngine::CalculateBoneMatrices(GameObject& obj, float time) {
    const AnimationClip& clip = obj.animations[obj.currentAnimationIndex];
    const Skeleton& skeleton = obj.mesh.skeleton;
    
    if (obj.boneMatrices.size() != skeleton.bones.size()) {
        obj.boneMatrices.resize(skeleton.bones.size(), glm::mat4(1.0f));
    }

    
    CalculateHierarchy(obj, 0, glm::mat4(1.0f), clip, time);
}

void AniEngine::CalculateHierarchy(GameObject& obj, int boneIdx, const glm::mat4& parentTransform, const AnimationClip& clip, float time) {
    const Skeleton& skeleton = obj.mesh.skeleton;
    if (boneIdx < 0 || boneIdx >= (int)skeleton.bones.size()) return;

    const Bone& bone = skeleton.bones[boneIdx];
    glm::mat4 localTransform = bone.localTransform;

    
    for (const auto& channel : clip.channels) {
        if (channel.boneName == bone.name) {
            glm::vec3 pos = InterpolatePosition(channel, time);
            glm::quat rot = InterpolateRotation(channel, time);
            glm::vec3 scale = InterpolateScale(channel, time);

            localTransform = glm::translate(glm::mat4(1.0f), pos) * 
                             glm::toMat4(rot) * 
                             glm::scale(glm::mat4(1.0f), scale);
            break;
        }
    }

    glm::mat4 globalTransform = parentTransform * localTransform;
    obj.boneMatrices[boneIdx] = globalTransform * bone.offsetMatrix;

    
    for (int i = 0; i < (int)skeleton.bones.size(); i++) {
        if (skeleton.bones[i].parentIndex == boneIdx) {
            CalculateHierarchy(obj, i, globalTransform, clip, time);
        }
    }
}

glm::vec3 AniEngine::InterpolatePosition(const AnimationChannel& channel, float time) {
    if (channel.positionKeys.empty()) return glm::vec3(0.0f);
    if (channel.positionKeys.size() == 1) return channel.positionKeys[0].value;

    int idx = 0;
    for (int i = 0; i < (int)channel.positionKeys.size() - 1; i++) {
        if (time < channel.positionKeys[i+1].time) {
            idx = i;
            break;
        }
    }

    const auto& k1 = channel.positionKeys[idx];
    const auto& k2 = channel.positionKeys[idx+1];
    float t = (time - k1.time) / (k2.time - k1.time);
    return glm::mix(k1.value, k2.value, t);
}

glm::quat AniEngine::InterpolateRotation(const AnimationChannel& channel, float time) {
    if (channel.rotationKeys.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (channel.rotationKeys.size() == 1) return channel.rotationKeys[0].value;

    int idx = 0;
    for (int i = 0; i < (int)channel.rotationKeys.size() - 1; i++) {
        if (time < channel.rotationKeys[i+1].time) {
            idx = i;
            break;
        }
    }

    const auto& k1 = channel.rotationKeys[idx];
    const auto& k2 = channel.rotationKeys[idx+1];
    float t = (time - k1.time) / (k2.time - k1.time);
    return glm::slerp(k1.value, k2.value, t);
}

glm::vec3 AniEngine::InterpolateScale(const AnimationChannel& channel, float time) {
    if (channel.scaleKeys.empty()) return glm::vec3(1.0f);
    if (channel.scaleKeys.size() == 1) return channel.scaleKeys[0].value;

    int idx = 0;
    for (int i = 0; i < (int)channel.scaleKeys.size() - 1; i++) {
        if (time < channel.scaleKeys[i+1].time) {
            idx = i;
            break;
        }
    }

    const auto& k1 = channel.scaleKeys[idx];
    const auto& k2 = channel.scaleKeys[idx+1];
    float t = (time - k1.time) / (k2.time - k1.time);
    return glm::mix(k1.value, k2.value, t);
}
