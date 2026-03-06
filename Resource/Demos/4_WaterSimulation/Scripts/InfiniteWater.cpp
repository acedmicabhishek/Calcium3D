
#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Scene.h"          
#include "Camera.h"         
#include <glm/glm.hpp>
#include <cmath>

class InfiniteWater : public Behavior {
public:
    
    
    float snapCell = 5000.0f;

    void OnStart() override {
        
        SetScale(20000.0f, 1.0f, 20000.0f);
    }

    void OnUpdate(float) override {
        Camera* cam = GetCamera();
        if (!cam || !gameObject) return;

        
        
        if (gameObject->scale.x < 19000.0f || gameObject->scale.x > 21000.0f) {
            SetScale(20000.0f, 1.0f, 20000.0f);
        }

        
        float snappedX = std::floor(cam->Position.x / snapCell) * snapCell;
        float snappedZ = std::floor(cam->Position.z / snapCell) * snapCell;

        SetPosition(snappedX, gameObject->position.y, snappedZ);
    }
};

REGISTER_BEHAVIOR(InfiniteWater)
