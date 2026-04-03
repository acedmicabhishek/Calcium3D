#ifndef C3D_SDF_H
#define C3D_SDF_H

#include <glm/glm.hpp>

struct GameObject;

namespace C3D {
    namespace SDF {
        void SetEnabled(GameObject* obj, bool enabled);
        void SetResolution(GameObject* obj, int resolution);
        void SetBounds(GameObject* obj, const glm::vec3& minP, const glm::vec3& maxP);
        void Generate(GameObject* obj);
    }
}

#endif
