#ifndef ANI_ENGINE_H
#define ANI_ENGINE_H

#include "Animation.h"
#include "../Scene/Scene.h"
#include <vector>
#include <glm/glm.hpp>

class AniEngine {
public:
    static void Update(GameObject& obj, float dt);

private:
    static void CalculateBoneMatrices(GameObject& obj, float time);
    static void CalculateHierarchy(GameObject& obj, int boneIdx, const glm::mat4& parentTransform, const AnimationClip& clip, float time);
    
    static glm::vec3 InterpolatePosition(const AnimationChannel& channel, float time);
    static glm::quat InterpolateRotation(const AnimationChannel& channel, float time);
    static glm::vec3 InterpolateScale(const AnimationChannel& channel, float time);
};

#endif
