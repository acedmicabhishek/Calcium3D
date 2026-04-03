#ifndef C3D_LIGHT_H
#define C3D_LIGHT_H

#include <glm/glm.hpp>

namespace C3D {
    namespace Light {
        void SetPointLightColor(int index, const glm::vec4& color);
        void SetPointLightIntensity(int index, float intensity);
        void SetPointLightPosition(int index, const glm::vec3& position);
        void SetPointLightEnabled(int index, bool enabled);
        void SetPointLightCastShadows(int index, bool castShadows);
        
        int GetPointLightCount();
        int CreatePointLight(const glm::vec3& position, const glm::vec4& color, float intensity);
    }
}

#endif
