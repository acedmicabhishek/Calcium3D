#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <glm/glm.hpp>
#include <vector>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}
};

struct GameObject; 

class PhysicsEngine {
public:
    static bool GlobalGravityEnabled;
    static glm::vec3 Gravity;
    static glm::vec3 GlobalAcceleration;
    
    static bool GlobalPhysicsEnabled;
    static bool ImpulseEnabled;
    static int SubSteps;
    static float LinearDamping;
    static float AngularDamping;
    static float GlobalAirResistance;
    static bool GlobalCOMEnabled;

    
    static void Update(float deltaTime, float time, std::vector<GameObject>& objects);

    
    static bool CheckCollision(const AABB& a, const AABB& b);

    
    static AABB GetTransformedAABB(const AABB& localAABB, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

    struct RaycastHit {
        GameObject* object;
        glm::vec3 point;
        glm::vec3 normal;
        float distance;
    };

    static bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, std::vector<GameObject>& objects, RaycastHit& outHit);
};

#endif
