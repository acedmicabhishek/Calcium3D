#ifndef C3D_PHYSICS_H
#define C3D_PHYSICS_H

#include <glm/glm.hpp>

struct GameObject;

namespace C3D {
    namespace Physics {
        void AddForce(GameObject* obj, const glm::vec3& force);
        void AddImpulse(GameObject* obj, const glm::vec3& impulse);
        void AddTorque(GameObject* obj, const glm::vec3& torque);
        
        void SetVelocity(GameObject* obj, const glm::vec3& velocity);
        glm::vec3 GetVelocity(GameObject* obj);
        
        void SetAngularVelocity(GameObject* obj, const glm::vec3& angularVelocity);
        glm::vec3 GetAngularVelocity(GameObject* obj);
        
        void SetMass(GameObject* obj, float mass);
        float GetMass(GameObject* obj);
        
        void SetFriction(GameObject* obj, float friction);
        void SetBounciness(GameObject* obj, float restitution);
        
        void SetGravityEnabled(GameObject* obj, bool enabled);
        void SetCollisionEnabled(GameObject* obj, bool enabled);
        void SetStatic(GameObject* obj, bool isStatic);
        void SetTrigger(GameObject* obj, bool isTrigger);

        struct RaycastHit {
            GameObject* object;
            glm::vec3 point;
            glm::vec3 normal;
            float distance;
        };

        bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit);

        namespace Global {
            void SetGravity(const glm::vec3& gravity);
            void SetAirResistance(float resistance);
            void SetSubSteps(int subSteps);
            void SetPhysicsEnabled(bool enabled);
        }
    }
}

#endif
