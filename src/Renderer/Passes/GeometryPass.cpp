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
        shader.setInt("pointLightCount", (int)pointLights.size());
        for (int i = 0; i < (int)pointLights.size() && i < 16; ++i) {
             std::string base = "pointLights[" + std::to_string(i) + "]";
             shader.setVec3(base + ".position", pointLights[i].position);
             shader.setVec4(base + ".color", pointLights[i].color);
             shader.setFloat(base + ".intensity", pointLights[i].intensity);
             shader.setFloat(base + ".constant", pointLights[i].constant);
             shader.setFloat(base + ".linear", pointLights[i].linear);
             shader.setFloat(base + ".quadratic", pointLights[i].quadratic);
        }
        
        
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
        
        
        Renderer::RenderScene(*context.scene, *context.camera, shader, context.globalTilingFactor, context.renderEditorObjects, context.deltaTime);
    }
}
