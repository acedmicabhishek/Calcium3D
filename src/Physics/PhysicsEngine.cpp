#include "PhysicsEngine.h"
#include "../Scene/Scene.h"

bool PhysicsEngine::GlobalGravityEnabled = true;
glm::vec3 PhysicsEngine::Gravity = glm::vec3(0.0f, -9.81f, 0.0f);

bool PhysicsEngine::CheckCollision(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

AABB PhysicsEngine::GetTransformedAABB(const AABB& localAABB, const glm::vec3& position, const glm::vec3& scale) {
    return AABB(
        (localAABB.min * scale) + position,
        (localAABB.max * scale) + position
    );
}

void PhysicsEngine::Update(float deltaTime, std::vector<GameObject>& objects) {
    
    
    
    for (auto& obj : objects) {
        if (obj.isStatic) continue;

        if (GlobalGravityEnabled && obj.useGravity) {
            obj.velocity += Gravity * deltaTime;
        }

        
        obj.velocity += obj.acceleration * deltaTime;
        
        
        obj.velocity *= 0.98f;
    }

    
    
    for (size_t i = 0; i < objects.size(); ++i) {
        auto& objA = objects[i];
        if (objA.isStatic) continue;

        
        glm::vec3 predictedPos = objA.position + (objA.velocity * deltaTime);
        AABB objAABB = GetTransformedAABB(objA.collider, predictedPos, objA.scale);

        bool collided = false;
        
        for (size_t j = 0; j < objects.size(); ++j) {
            if (i == j) continue;
            
            auto& objB = objects[j];
            AABB otherAABB = GetTransformedAABB(objB.collider, objB.position, objB.scale);

            if (CheckCollision(objAABB, otherAABB)) {
                
                
                collided = true;
                
                
                if (objA.velocity.y < 0.0f) {
                    
                    float localBottomOffset = objA.collider.min.y * objA.scale.y; 
                    objA.position.y = otherAABB.max.y - localBottomOffset + 0.001f;
                    objA.velocity.y = 0.0f;
                } else {
                    
                    objA.velocity = glm::vec3(0.0f);
                }
            }
        }

        if (!collided) {
            objA.position = predictedPos;
        }
    }
}
