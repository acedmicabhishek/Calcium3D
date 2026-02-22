#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Mesh.h"
#include "../Physics/PhysicsEngine.h"
#include "Behavior.h"
#include "../Renderer/Material.h"
#include <memory>

enum class ColliderShape { Box, Sphere };

struct GameObject {
    Mesh mesh; 
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    std::string name;
    
    ColliderShape shape = ColliderShape::Box;
    float collisionRadius = 0.5f;
    
    
    bool useGravity = false;
    bool isStatic = false;
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.5f;
    bool enableCollision = true;
    glm::vec3 centerOfMassOffset = glm::vec3(0.0f);

    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 angularVelocity;
    glm::vec3 torque;
    AABB collider;
    
    std::vector<std::shared_ptr<Behavior>> behaviors;
    std::vector<std::string> scriptNames; 
    Material material; 
    
    void ApplyImpulse(const glm::vec3& impulse) {
        if (!isStatic && mass > 0.0f) {
            velocity += impulse / mass;
        }
    }
    
    GameObject(Mesh m, const std::string& n = "Object") 
        : mesh(std::move(m)), position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f), name(n),
          shape(ColliderShape::Box), collisionRadius(0.5f),
          useGravity(false), isStatic(false), mass(1.0f), friction(0.5f), restitution(0.5f), enableCollision(true),
          centerOfMassOffset(0.0f), velocity(0.0f), acceleration(0.0f), angularVelocity(0.0f), torque(0.0f) {
              
        if (!mesh.vertices.empty()) {
            glm::vec3 minExtent = mesh.vertices[0].position;
            glm::vec3 maxExtent = mesh.vertices[0].position;
            for (const auto& v : mesh.vertices) {
                minExtent = glm::min(minExtent, v.position);
                maxExtent = glm::max(maxExtent, v.position);
            }
            collider = AABB(minExtent, maxExtent);
        }
    }

};

class Scene {
public:
    Scene();
    ~Scene();

    void Update(float dt);
    
    void AddObject(GameObject object);
    void RemoveObject(int index);
    void Clear();
    
    std::vector<GameObject>& GetObjects() { return m_Objects; }
    
    
    struct Light {
        glm::vec3 position;
        glm::vec4 color;
        float intensity;
        bool enabled;
    };
    struct PointLight {
        glm::vec3 position;
        glm::vec4 color;
        float intensity;
        bool enabled;
        
        float constant;
        float linear;
        float quadratic;
    };
    
    
    PointLight* CreatePointLight();
    void RemovePointLight(int index);
    std::vector<PointLight>& GetPointLights() { return m_PointLights; }

    void Save(const std::string& path);
    void Load(const std::string& path);

    const std::string& GetFilepath() const { return m_Filepath; }
    void SetFilepath(const std::string& path) { m_Filepath = path; }

private:
    std::string m_Filepath = "";
    std::vector<GameObject> m_Objects;
    std::vector<PointLight> m_PointLights;
    
public:
    PhysicsEngine physicsEngine;
};

#endif
