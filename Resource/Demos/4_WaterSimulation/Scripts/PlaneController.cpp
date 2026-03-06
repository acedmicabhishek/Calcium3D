
#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Scene.h"
#include "Camera.h"
#include "Core/InputManager.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

class PlaneController : public Behavior {
public:
    
    float throttleAccel   = 18.0f;    
    float dragCoeff       = 0.4f;     
    float minSpeed        = 5.0f;     
    float maxSpeed        = 120.0f;   
    float cruiseAlt       = 15.0f;    
    float altHoldForce    = 4.0f;     
    float rollSpeed       = 80.0f;    
    float rollYawCoupling = 0.4f;     
    float pitchSpeed      = 45.0f;    
    float autoLevelRate   = 1.2f;     
    float boostMult       = 2.0f;     
    float camDistance     = 8.5f;     
    float camHeight       = 3.0f;     
    float camLagFactor    = 4.0f;     
    float bankVisualMult  = 1.5f;     

    
    float currentSpeed    = 40.0f;   
    float currentRoll     = 0.0f;    
    float currentPitch    = 0.0f;    
    float currentYaw      = 0.0f;    
    glm::vec3 camSmoothPos = glm::vec3(0.0f);
    bool camInitialized   = false;

    void OnStart() override {
        if (!gameObject) return;
        
        glm::vec3 euler = glm::degrees(glm::eulerAngles(gameObject->rotation));
        currentPitch = euler.x;
        currentYaw   = euler.y;
        currentRoll  = euler.z;
        currentSpeed = 40.0f;
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        Camera* cam = GetCamera();

        
        bool wDown = InputManager::IsKeyPressed(GLFW_KEY_W);
        bool sDown = InputManager::IsKeyPressed(GLFW_KEY_S);
        bool aDown = InputManager::IsKeyPressed(GLFW_KEY_A);
        bool dDown = InputManager::IsKeyPressed(GLFW_KEY_D);
        bool qDown = InputManager::IsKeyPressed(GLFW_KEY_Q);
        bool eDown = InputManager::IsKeyPressed(GLFW_KEY_E);
        bool shiftDown = InputManager::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);

        
        float targetSpeed = 40.0f; 
        if (wDown) {
            targetSpeed = 120.0f; 
            if (shiftDown) targetSpeed = 260.0f; 
        }
        else if (sDown) {
            targetSpeed = 5.0f; 
        }

        
        float accelRate = (targetSpeed > currentSpeed) ? 75.0f : 100.0f; 
        currentSpeed += (targetSpeed - currentSpeed) * dt * (accelRate / 60.0f);
        
        
        currentSpeed -= currentSpeed * 0.05f * dt;
        currentSpeed = glm::clamp(currentSpeed, 5.0f, 260.0f);

        
        SetProperty("aircraft.speed", currentSpeed);
        SetProperty("aircraft.altitude", gameObject->position.y);
        SetProperty("aircraft.afterburner", shiftDown && wDown);

        
        float rollInput  = 0.0f;
        if (aDown) rollInput =  1.0f;
        if (dDown) rollInput = -1.0f;
        currentRoll += rollInput * 100.0f * dt; 

        
        if (!aDown && !dDown) {
            currentRoll *= (1.0f - 2.5f * dt);
        }
        currentRoll = glm::clamp(currentRoll, -80.0f, 80.0f);

        
        float pitchInput = 0.0f;
        if (qDown) pitchInput = -1.0f;
        if (eDown) pitchInput =  1.0f;

        
        float altError = cruiseAlt - gameObject->position.y;
        float altPitchCorrect = glm::clamp(altError * 0.4f, -20.0f, 20.0f);

        float targetPitch = pitchInput * 65.0f + altPitchCorrect;
        currentPitch += (targetPitch - currentPitch) * dt * 2.5f;
        currentPitch = glm::clamp(currentPitch, -60.0f, 60.0f);

        
        currentYaw += currentRoll * 0.7f * dt;

        
        glm::quat qYaw   = glm::angleAxis(glm::radians(currentYaw),   glm::vec3(0,1,0));
        glm::quat qPitch = glm::angleAxis(glm::radians(-currentPitch), glm::vec3(1,0,0));
        glm::quat qRoll  = glm::angleAxis(glm::radians(currentRoll * bankVisualMult), glm::vec3(0,0,1));
        glm::quat finalRot = qYaw * qPitch * qRoll;

        gameObject->rotation = finalRot;

        
        glm::vec3 forward = finalRot * glm::vec3(0.0f, 0.0f, -1.0f);
        gameObject->position += forward * currentSpeed * dt;

        
        if (cam) {
            glm::vec3 planePos   = gameObject->position;
            glm::vec3 backDir    = finalRot * glm::vec3(0.0f, 0.0f, 1.0f);  
            glm::vec3 upDir      = finalRot * glm::vec3(0.0f, 1.0f, 0.0f);

            glm::vec3 wantedCamPos = planePos
                + backDir   * camDistance
                + upDir     * camHeight;

            
            if (!camInitialized) {
                camSmoothPos = wantedCamPos;
                camInitialized = true;
            }

            
            camSmoothPos = glm::mix(camSmoothPos, wantedCamPos, dt * camLagFactor);

            cam->Position = camSmoothPos;

            
            glm::vec3 lookDir = glm::normalize(planePos - camSmoothPos);
            cam->Orientation = lookDir;
            cam->Up = glm::vec3(0, 1, 0);
        }
    }
};

REGISTER_BEHAVIOR(PlaneController)
