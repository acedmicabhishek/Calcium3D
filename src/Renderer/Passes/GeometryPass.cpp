#include "GeometryPass.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Camera.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../Tools/Profiler/Profiler.h"
#include "../Tools/Profiler/GpuProfiler.h"

GeometryPass::GeometryPass()
{
    m_Name = "GeometryPass";
}

void GeometryPass::Init()
{
    
    
}

void GeometryPass::Execute(const RenderContext& context)
{
    PROFILE_SCOPE("GeometryPass");
    GPU_PROFILE_SCOPE("GeometryPass");
    if (context.msaaSamples > 0) {
        if (context.msaaGeometryPass)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);
    }
    
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); 
    
    
    Shader& shader = ResourceManager::GetShader("default");
    shader.use();
    
    
    shader.setVec3("camPos", context.camera->Position);
    
    if (context.scene) {
        
        auto& pointLights = context.scene->GetPointLights();
        int shadowCasters = 0;
        int activeLights = 0;
        for (int i = 0; i < (int)pointLights.size() && activeLights < 16; ++i) {
             if (!pointLights[i].enabled) continue;
             
             std::string base = "pointLights[" + std::to_string(activeLights) + "]";
             shader.setVec3(base + ".position", pointLights[i].position);
             shader.setVec4(base + ".color", pointLights[i].color);
             shader.setFloat(base + ".intensity", pointLights[i].intensity);
             shader.setFloat(base + ".constant", pointLights[i].constant);
             shader.setFloat(base + ".linear", pointLights[i].linear);
             shader.setFloat(base + ".quadratic", pointLights[i].quadratic);
             
             int sIdx = -1;
             if (pointLights[i].castShadows && shadowCasters < 4) {
                 sIdx = shadowCasters;
                 shadowCasters++;
             }
             shader.setInt(base + ".shadowIndex", sIdx);
             
             activeLights++;
        }
        shader.setInt("pointLightCount", activeLights);
        
        
        shader.setVec3("sunLight.direction", context.sunPosition);
        shader.setVec3("sunLight.color", glm::vec3(context.sunColor));
        float sunHeight = context.sunPosition.y;
        float sunInt = context.sunIntensity * glm::smoothstep(-2.0f, 2.0f, sunHeight);
        if (!context.sunEnabled) sunInt = 0.0f;
        shader.setFloat("sunLight.intensity", sunInt);
        
        
        shader.setVec3("moonLight.direction", context.moonPosition);
        shader.setVec3("moonLight.color", glm::vec3(context.moonColor));
        float moonHeight = context.moonPosition.y;
        float moonInt = context.moonIntensity * glm::smoothstep(-2.0f, 2.0f, moonHeight);
        if (!context.moonEnabled) moonInt = 0.0f;
        shader.setFloat("moonLight.intensity", moonInt);

        
        shader.setInt("enableShadows", context.enableShadows ? 1 : 0);
        shader.setInt("enablePointShadows", context.enablePointShadows ? 1 : 0);
        shader.setFloat("shadowBias", context.shadowBias);
        shader.setFloat("pointShadowFarPlane", context.pointShadowFarPlane);
        
        if (context.enableShadows) {
            shader.setMat4("lightSpaceMatrix", context.lightSpaceMatrix);
            glActiveTexture(GL_TEXTURE4); 
            glBindTexture(GL_TEXTURE_2D, context.dirShadowMap);
            shader.setInt("dirShadowMap", 4);
        }
        
        for (int i = 0; i < 4; i++) {
            glActiveTexture(GL_TEXTURE5 + i); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, context.pointShadowCubemaps[i]);
            shader.setInt("pointShadowMap" + std::to_string(i), 5 + i);
        }
        
        Renderer::RenderScene(*context.scene, *context.camera, shader, context.globalTilingFactor, context.renderEditorObjects, context.deltaTime, context.time, 1);
    }
}
