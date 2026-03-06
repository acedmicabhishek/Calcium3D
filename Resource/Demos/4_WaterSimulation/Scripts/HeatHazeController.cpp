#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Core/ResourceManager.h"
#include "Scene.h"
#include "Scene/ObjectFactory.h"
#include <glm/glm.hpp>

class HeatHazeController : public Behavior {
public:
    float intensityState = 0.0f;
    float timeTracker = 0.0f;

    void OnStart() override {
        
        if (!ResourceManager::HasShader("heathaze")) {
            ResourceManager::LoadShader("heathaze", 
                "Shaders/passes/effects/heathaze.vert", 
                "Shaders/passes/effects/heathaze.frag");
        }
        
        if (gameObject) {
            gameObject->material.customShaderName = "heathaze";
            gameObject->material.isTransparent = true; 
            
            
            if (gameObject->meshType == MeshType::None) {
                gameObject->meshType = MeshType::Plane;
                gameObject->mesh = ObjectFactory::createPlane();
            }
        }
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        timeTracker += dt;

        
        auto propThrottle = GetProperty("aircraft.speed");
        auto propAB = GetProperty("aircraft.afterburner");

        float throttle = 0.0f;
        if (propThrottle.has_value()) throttle = std::any_cast<float>(propThrottle);
        
        bool afterburner = false;
        if (propAB.has_value()) afterburner = std::any_cast<bool>(propAB);

        
        float targetIntensity = 0.1f + (throttle / 260.0f) * 0.9f;
        
        if (afterburner) targetIntensity *= 2.0f;
        if (throttle < 5.0f) targetIntensity = 0.0f; 

        targetIntensity = glm::clamp(targetIntensity, 0.0f, 2.0f);

        
        intensityState += (targetIntensity - intensityState) * dt * 5.0f;
        
        
        gameObject->isActive = (intensityState > 0.01f);

        
        float scaleZ = 1.0f + intensityState * 3.0f;
        gameObject->scale.z = scaleZ;

        
        if (ResourceManager::HasShader("heathaze")) {
            Shader& shader = ResourceManager::GetShader("heathaze");
            shader.use();
            shader.setFloat("time", timeTracker);
            shader.setFloat("intensity", intensityState);
        }
    }
};

REGISTER_BEHAVIOR(HeatHazeController)
