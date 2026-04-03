#ifndef C3D_GRAPHICS_H
#define C3D_GRAPHICS_H

#include <glm/glm.hpp>
#include <string>

struct GameObject;

namespace C3D {
    namespace Graphics {
        
        void SetAlbedo(GameObject* obj, const glm::vec3& color);
        void SetMetallic(GameObject* obj, float metallic);
        void SetRoughness(GameObject* obj, float roughness);
        void SetAO(GameObject* obj, float ao);
        void SetShininess(GameObject* obj, float shininess);
        
        void SetEmissive(GameObject* obj, bool enabled, float intensity, const glm::vec3& color);
        void SetEmissionIntensity(GameObject* obj, float intensity);
        
        void SetTexture(GameObject* obj, const std::string& path);
        void SetCustomShader(GameObject* obj, const std::string& shaderName);
        void SetTransparent(GameObject* obj, bool enabled);
        
        
        void SetBrightness(GameObject* obj, float brightness);
        void SetKeepAspect(GameObject* obj, bool enabled);
        
        
        void SetSunBloom(float intensity);
        void SetMoonBloom(float intensity);
        void SetSSR(bool enabled);
        void SetSSM(bool enabled); 
    }
}

#endif
