#include "GeometryPass.h"
#include "../ClusteredLighting.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "Camera.h"
#include "Frustum.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

GeometryPass::GeometryPass() { m_Name = "GeometryPass"; }

void GeometryPass::Init() {
  if (!ResourceManager::HasShader("default")) {
    ResourceManager::LoadShader("default",
                                "shaders/passes/geometry/default.vert",
                                "shaders/passes/geometry/default.frag");
  }
  if (!ResourceManager::HasShader("clustered_forward")) {
    ResourceManager::LoadShader(
        "clustered_forward", "shaders/passes/geometry/default.vert",
        "shaders/passes/geometry/clustered_forward.frag");
  }

  m_DefaultShader = &ResourceManager::GetShader("default");
}

void GeometryPass::Execute(const RenderContext &context) {
  PROFILE_SCOPE("GeometryPass");
  GPU_PROFILE_SCOPE("GeometryPass");
  if (context.msaaSamples > 0) {
    if (context.msaaGeometryPass)
      glEnable(GL_MULTISAMPLE);
    else
      glDisable(GL_MULTISAMPLE);
  }

  glEnable(GL_DEPTH_TEST);
  if (context.zPrepass) {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE); 
  } else {
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
  }
  
  
  

  Shader *activeShader = m_DefaultShader;
  activeShader->use();

  activeShader->setVec3("camPos", context.camera->Position);

  if (context.scene) {

    auto &pointLights = context.scene->GetPointLights();
    int shadowCasters = 0;
    int activePointLights = 0;

    
    Frustum camFrustum =
        Frustum::CreateFrustumFromCamera(context.camera->GetProjectionMatrix() *
                                         context.camera->GetViewMatrix());

    for (int i = 0; i < (int)pointLights.size() && activePointLights < 16;
         ++i) {
      if (!pointLights[i].enabled)
        continue;

      
      
      float maxRange = 0.0f;
      float q = pointLights[i].quadratic;
      float l = pointLights[i].linear;
      float c = pointLights[i].constant - 256.0f * pointLights[i].intensity;
      if (q > 0.0001f) {
        float disc = l * l - 4.0f * q * c;
        if (disc > 0.0f)
          maxRange = (-l + sqrtf(disc)) / (2.0f * q);
        else
          maxRange = 50.0f; 
      } else if (l > 0.0001f) {
        maxRange = -c / l;
      } else {
        maxRange = 200.0f; 
      }
      maxRange = glm::max(maxRange, 1.0f);

      
      
      bool isCulled = false;
      if (context.lightCulling) {
        isCulled =
            !camFrustum.IsSphereOnFrustum(pointLights[i].position, maxRange);
      }

      if (!isCulled) {
        std::string base =
            "pointLights[" + std::to_string(activePointLights) + "]";
        m_DefaultShader->setVec3(base + ".position", pointLights[i].position);
        m_DefaultShader->setVec4(base + ".color", pointLights[i].color);
        m_DefaultShader->setFloat(base + ".intensity",
                                  pointLights[i].intensity);
        m_DefaultShader->setFloat(base + ".constant", pointLights[i].constant);
        m_DefaultShader->setFloat(base + ".linear", pointLights[i].linear);
        m_DefaultShader->setFloat(base + ".quadratic",
                                  pointLights[i].quadratic);

        int sIdx = -1;
        if (pointLights[i].castShadows && shadowCasters < 4) {
          sIdx = shadowCasters;
          shadowCasters++;
        }
        m_DefaultShader->setInt(base + ".shadowIndex", sIdx);

        if (context.clusteredShading &&
            ResourceManager::HasShader("clustered_forward")) {
          
          
          
        }
        activePointLights++;
      }
    }
    m_DefaultShader->setInt("pointLightCount", activePointLights);

    if (context.clusteredShading) {
      if (ResourceManager::HasShader("clustered_forward")) {
        activeShader = &ResourceManager::GetShader("clustered_forward");
        activeShader->use();

        ClusteredLighting::UpdateClusters(*context.camera, *context.scene);
        ClusteredLighting::BindBuffers(0);

        activeShader->setFloat("zNear", context.camera->nearPlane);
        activeShader->setFloat("zFar", context.camera->farPlane);
        activeShader->setVec2("screenSize",
                              glm::vec2(context.width, context.height));
      }
    }

    activeShader->setVec3("sunLight.direction", context.sunPosition);
    activeShader->setVec3("sunLight.color", glm::vec3(context.sunColor));
    float sunHeight = context.sunPosition.y;
    float sunInt =
        context.sunIntensity * glm::smoothstep(-2.0f, 2.0f, sunHeight);
    if (!context.sunEnabled)
      sunInt = 0.0f;
    activeShader->setFloat("sunLight.intensity", sunInt);

    activeShader->setVec3("moonLight.direction", context.moonPosition);
    activeShader->setVec3("moonLight.color", glm::vec3(context.moonColor));
    float moonHeight = context.moonPosition.y;
    float moonInt =
        context.moonIntensity * glm::smoothstep(-2.0f, 2.0f, moonHeight);
    if (!context.moonEnabled)
      moonInt = 0.0f;
    activeShader->setFloat("moonLight.intensity", moonInt);

    activeShader->setInt("enableShadows", context.enableShadows ? 1 : 0);
    activeShader->setInt("enablePointShadows",
                         context.enablePointShadows ? 1 : 0);
    activeShader->setFloat("shadowBias", context.shadowBias);
    activeShader->setFloat("pointShadowFarPlane", context.pointShadowFarPlane);

    if (context.enableShadows) {
      activeShader->setMat4("lightSpaceMatrix", context.lightSpaceMatrix);
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, context.dirShadowMap);
      activeShader->setInt("dirShadowMap", 4);
    }

    for (int i = 0; i < 4; i++) {
      glActiveTexture(GL_TEXTURE5 + i);
      glBindTexture(GL_TEXTURE_CUBE_MAP, context.pointShadowCubemaps[i]);
      activeShader->setInt("pointShadowMap" + std::to_string(i), 5 + i);
    }

    
    activeShader->setBool("debugZPrepass",
                          Renderer::s_VisualizeZPrepass && context.zPrepass);
    activeShader->setBool("debugVRS", Renderer::s_VisualizeVRS);

    
    if (context.vrs) {
      
      
      glEnable(0x9563); 
    }
    
    if (context.wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    Renderer::RenderScene(
        *context.scene, *context.camera, *activeShader,
        context.globalTilingFactor, context.renderEditorObjects,
        context.deltaTime, context.time, 1, context.cullingCamera,
        context.objCulling, context.backfaceCulling,
        context.materialOptimisation, context.visualizeCulling, context.autoLOD,
        context.zPrepass, context.staticBatching, context.dynamicBatching);

    
    if (context.wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (context.vrs) {
      glDisable(0x9563); 
    }

    
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
  }
}
