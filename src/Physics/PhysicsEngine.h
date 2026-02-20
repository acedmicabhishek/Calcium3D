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

    
    static void Update(float deltaTime, std::vector<GameObject>& objects);

    
    static bool CheckCollision(const AABB& a, const AABB& b);

    
    static AABB GetTransformedAABB(const AABB& localAABB, const glm::vec3& position, const glm::vec3& scale);
};

#endif
