#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Core/ResourceManager.h"
#include "Scene.h"
#include <glm/glm.hpp>

#include "Scene/ObjectFactory.h"

class ThrusterController : public Behavior {
public:
    float throttle = 0.0f;
    float timeTracker = 0.0f;

    void OnStart() override {
        
        if (!ResourceManager::HasShader("thruster")) {
            ResourceManager::LoadShader("thruster", 
                "Shaders/passes/effects/thruster.vert", 
                "Shaders/passes/effects/thruster.frag");
        }
        
        if (gameObject) {
            gameObject->material.customShaderName = "thruster";
            gameObject->material.isTransparent = true; 
            
            
            if (gameObject->meshType == MeshType::None) {
                gameObject->meshType = MeshType::Cube;
                gameObject->mesh = ObjectFactory::createCube();
            }
        }
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;

        
        auto propAB = GetProperty("aircraft.afterburner");
        auto propSpeed = GetProperty("aircraft.speed");
        
        bool isAB = false;
        float speed = 0.0f;
        
        if (propAB.has_value()) isAB = std::any_cast<bool>(propAB);
        if (propSpeed.has_value()) speed = std::any_cast<float>(propSpeed);

        
        
        float targetThrottle = isAB ? 1.5f : (speed > 40.0f ? 0.4f : 0.0f);
        throttle += (targetThrottle - throttle) * dt * 4.0f;

        
        gameObject->isActive = (throttle > 0.01f);

        if (ResourceManager::HasShader("thruster")) {
            Shader& shader = ResourceManager::GetShader("thruster");
            shader.use();
            shader.setFloat("intensity", throttle);
        }

        
        float lengthPulse = 1.0f + 0.15f * sin(GetSceneTime() * 25.0f) * throttle;
        gameObject->scale = glm::vec3(1.2f, lengthPulse * (isAB ? 4.5f : 2.0f), 1.2f);
    }

    
    double GetSceneTime() {
        
        static double totalTime = 0;
        totalTime += 0.016; 
        
        
        return totalTime; 
    }
};

REGISTER_BEHAVIOR(ThrusterController)
