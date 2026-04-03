#ifndef C3D_MATH_H
#define C3D_MATH_H

#include <glm/glm.hpp>
#include <random>

namespace C3D {
    namespace Math {
        float RandomRange(float min, float max);
        int RandomInt(int min, int max);
        
        float Lerp(float a, float b, float t);
        glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
        
        float Clamp(float val, float min, float max);
        int Clamp(int val, int min, int max);
        
        float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime);
        glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, float smoothTime, float deltaTime);
    }
}

#endif
