#ifndef C3D_ENVIRONMENT_H
#define C3D_ENVIRONMENT_H

#include <glm/glm.hpp>

namespace C3D {
    namespace Environment {
        void SetTimeOfDay(float time);
        float GetTimeOfDay();
        void SetTimeSpeed(float speed);
        
        void SetSkyboxEnabled(bool enabled);
        void SetCloudsEnabled(bool enabled);
        void SetWaterEnabled(bool enabled);
        
        void SetAmbientColor(const glm::vec3& color);
        void SetAmbientIntensity(float intensity);
        
        void SetFogDensity(float density);
        void SetFogColor(const glm::vec3& color);
        
        void SetCloudDensity(float density);
        void SetCloudSpeed(float speed);
        void SetCloudHeight(float height);
    }
}

#endif
