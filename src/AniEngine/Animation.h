#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Bone {
    std::string name;
    int index = -1;
    int parentIndex = -1;
    glm::mat4 offsetMatrix = glm::mat4(1.0f); 
    glm::mat4 localTransform = glm::mat4(1.0f);
};

struct Skeleton {
    std::vector<Bone> bones;
    std::map<std::string, int> boneMapping;
};

struct VectorKey {
    float time;
    glm::vec3 value;
};

struct QuatKey {
    float time;
    glm::quat value;
};

struct AnimationChannel {
    std::string boneName;
    std::vector<VectorKey> positionKeys;
    std::vector<QuatKey> rotationKeys;
    std::vector<VectorKey> scaleKeys;
};

struct AnimationClip {
    std::string name;
    float duration = 0.0f;
    float ticksPerSecond = 0.0f;
    std::vector<AnimationChannel> channels;
};

#endif
