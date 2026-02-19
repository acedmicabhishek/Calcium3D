#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Mesh.h"

struct GameObject {
    Mesh mesh; 
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    std::string name;
    
    GameObject(Mesh m, const std::string& n = "Object") 
        : mesh(std::move(m)), position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f), name(n) {}

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

private:
    std::vector<GameObject> m_Objects;
    std::vector<PointLight> m_PointLights;
};

#endif
