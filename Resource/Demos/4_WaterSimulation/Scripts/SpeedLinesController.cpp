#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Core/ResourceManager.h"
#include "Scene.h"
#include "Scene/ObjectFactory.h"
#include <glm/glm.hpp>

class SpeedLinesController : public Behavior {
public:
    float intensityState = 0.0f;
    float timeTracker = 0.0f;

    void OnStart() override {
        if (!ResourceManager::HasShader("speedlines")) {
            ResourceManager::LoadShader("speedlines", 
                "Shaders/passes/effects/speedlines.vert", 
                "Shaders/passes/effects/speedlines.frag");
        }
        if (gameObject) {
            gameObject->material.customShaderName = "speedlines";
            gameObject->material.isTransparent = true; 
            
            
            if (gameObject->meshType == MeshType::None) {
                gameObject->meshType = MeshType::Cube;
                gameObject->mesh = ObjectFactory::createCube();
            }
        }
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        timeTracker += dt;

        
        auto propSpeed = GetProperty("aircraft.speed");
        float speed = 0.0f;
        if (propSpeed.has_value()) speed = std::any_cast<float>(propSpeed);

        
        
        float targetIntensity = 0.0f;
        if (speed > 45.0f) {
            targetIntensity = (speed - 45.0f) / 75.0f; 
        }
        targetIntensity = glm::clamp(targetIntensity, 0.0f, 1.0f);
        
        
        intensityState += (targetIntensity - intensityState) * dt * 5.0f;
        
        
        if (ResourceManager::HasShader("speedlines")) {
            Shader& shader = ResourceManager::GetShader("speedlines");
            shader.use();
            shader.setFloat("intensity", intensityState);
            shader.setFloat("time", timeTracker);
        }
    }
};

REGISTER_BEHAVIOR(SpeedLinesController)
