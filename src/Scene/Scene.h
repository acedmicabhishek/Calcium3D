#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Mesh.h"
#include "../Physics/PhysicsEngine.h"

struct GameObject {
    Mesh mesh; 
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    std::string name;
    
    
    bool useGravity = false;
    bool isStatic = false;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    AABB collider;
    
    GameObject(Mesh m, const std::string& n = "Object") 
        : mesh(std::move(m)), position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f), name(n),
          useGravity(false), isStatic(false), velocity(0.0f), acceleration(0.0f) {
              
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

private:
    std::vector<GameObject> m_Objects;
    std::vector<PointLight> m_PointLights;
    
public:
    PhysicsEngine physicsEngine;
};

#endif
