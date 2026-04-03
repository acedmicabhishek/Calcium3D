#ifndef C3D_WATER_H
#define C3D_WATER_H

#include <glm/glm.hpp>

struct GameObject;

namespace C3D {
    namespace Water {
        void SetWaveSpeed(GameObject* obj, float speed);
        void SetWaveStrength(GameObject* obj, float strength);
        void SetColor(GameObject* obj, const glm::vec3& color);
        void SetShininess(GameObject* obj, float shininess);
        void SetDensity(GameObject* obj, float density);
        void SetSurfaceHeight(GameObject* obj, float height);
        void SetDepth(GameObject* obj, float depth);
    }
}

#endif
