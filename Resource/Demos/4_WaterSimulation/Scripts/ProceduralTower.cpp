
#include "Behavior.h"
#include "BehaviorRegistry.h"
#include "Scene.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <cstdlib>

class ProceduralTower : public Behavior {
public:
    float cellSize         = 180.0f;  
    float spawnRange       = 600.0f;  
    float scaleBase        = 3.5f;    
    float scaleVariation   = 0.5f;    
    float popInDuration    = 0.6f;    
    float minDistFromCam   = 80.0f;   

    
    glm::vec3 lastPlacedCell = glm::vec3(1e9f);
    float popInTimer = 0.0f;
    bool  popping    = false;
    glm::vec3 targetScale = glm::vec3(1.0f);

    void OnStart() override {
        if (!gameObject) return;
        targetScale = gameObject->scale;
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        Camera* cam = GetCamera();
        if (!cam) return;

        glm::vec3 camXZ = glm::vec3(cam->Position.x, 0.0f, cam->Position.z);

        
        glm::vec3 bestCell = lastPlacedCell;
        float     bestDist = (lastPlacedCell.x > 1e8f) ? -1.0f : glm::length(camXZ - glm::vec3(lastPlacedCell.x, 0.0f, lastPlacedCell.z));

        
        int searchR = (int)(spawnRange / cellSize) + 1;
        int ci = (int)std::floor(camXZ.x / cellSize);
        int cj = (int)std::floor(camXZ.z / cellSize);

        for (int di = -searchR; di <= searchR; ++di) {
            for (int dj = -searchR; dj <= searchR; ++dj) {
                int ni = ci + di;
                int nj = cj + dj;
                
                float hx = (float)((ni * 1973 + nj * 9277) % 100) / 100.0f - 0.5f;
                float hz = (float)((ni * 5381 + nj * 3571) % 100) / 100.0f - 0.5f;
                glm::vec3 cellCenter = glm::vec3(
                    (float)ni * cellSize + hx * cellSize * 0.4f,
                    0.0f,
                    (float)nj * cellSize + hz * cellSize * 0.4f
                );
                float distFromCam = glm::length(camXZ - cellCenter);
                if (distFromCam < minDistFromCam || distFromCam > spawnRange) continue;

                
                if (bestDist < 0.0f || distFromCam < bestDist) {
                    bestCell = cellCenter;
                    bestDist = distFromCam;
                    
                    float sv = (float)((ni * 7681 + nj * 1031) % 100) / 100.0f;  
                    float finalScale = scaleBase + (sv - 0.5f) * 2.0f * scaleVariation;
                    targetScale = glm::vec3(finalScale, finalScale * 2.5f, finalScale);
                }
            }
        }

        
        glm::vec3 currentXZ = glm::vec3(gameObject->position.x, 0.0f, gameObject->position.z);
        float distToCurrentCell = glm::length(camXZ - currentXZ);

        if (distToCurrentCell > spawnRange * 1.1f && bestDist > 0.0f && bestCell != lastPlacedCell) {
            
            gameObject->position = glm::vec3(bestCell.x, 0.0f, bestCell.z);
            gameObject->scale    = glm::vec3(0.01f, 0.01f, 0.01f);  
            lastPlacedCell = bestCell;
            popInTimer = 0.0f;
            popping    = true;
        }

        
        if (popping) {
            popInTimer += dt;
            float t = glm::clamp(popInTimer / popInDuration, 0.0f, 1.0f);
            
            float ease = 1.0f - std::pow(1.0f - t, 3.0f);
            gameObject->scale = glm::mix(glm::vec3(0.01f), targetScale, ease);
            if (t >= 1.0f) {
                popping = false;
                gameObject->scale = targetScale;
            }
        }
    }
};

REGISTER_BEHAVIOR(ProceduralTower)
