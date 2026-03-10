#include "Renderer.h"
#include "AudioEngine/AudioEngine.h"
#include "Core/Logger.h"
#include "Core/ResourceManager.h"
#include "DynamicBatcher.h"
#include "Frustum.h"
#include "StaticBatcher.h"
#include "Tools/Profiler/GpuProfiler.h"
#include "Tools/Profiler/Profiler.h"
#include "VideoPlayer.h"

bool Renderer::s_BackfaceCulling = true;
bool Renderer::s_ObjFrustumCulling = true;
bool Renderer::s_LightFrustumCulling = true;
bool Renderer::s_ShadowFrustumCulling = true;
bool Renderer::s_MaterialOptimisation = false;
bool Renderer::s_ZPrepass = false;
bool Renderer::s_ShowCulledAsWireframe = true;
bool Renderer::s_VRS = false;
bool Renderer::s_VisualizeZPrepass = false;
bool Renderer::s_VisualizeVRS = false;
bool Renderer::s_AdaptiveShadowRes = true;
bool Renderer::s_StaticBatching = false;
bool Renderer::s_DynamicBatching = false;
bool Renderer::s_ClusteredShading = false;
bool Renderer::s_AutoLOD = true;
int Renderer::s_MaxFPS = 144;
bool Renderer::s_LowLatencyMode = false;

static GLuint s_boxVAO = 0;
static GLuint s_boxVBO = 0;
static GLuint s_boxEBO = 0;

void Renderer::Init() {
  glEnable(GL_DEPTH_TEST);

  
  bool vrsSupported = false;
  int numExtensions;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
  for (int i = 0; i < numExtensions; i++) {
    const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
    if (std::string(ext) == "GL_NV_shading_rate_image") {
      vrsSupported = true;
      break;
    }
  }

  if (vrsSupported) {
    Logger::AddLog("[Renderer] Variable Rate Shading (VRS) supported "
                   "(NV_shading_rate_image).");
  } else {
    Logger::AddLog(
        "[Renderer] Variable Rate Shading (VRS) not supported on this "
        "hardware.");
  }

  
  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, 0.5f,
      -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f, 0.5f,
  };

  unsigned int indices[] = {
      0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0,
      4, 1, 5, 2, 6, 3, 7, 0, 2, 1, 3, 4, 6, 5, 7 
                                                  
  };

  glGenVertexArrays(1, &s_boxVAO);
  glGenBuffers(1, &s_boxVBO);
  glGenBuffers(1, &s_boxEBO);

  glBindVertexArray(s_boxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, s_boxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_boxEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void Renderer::Shutdown() {}

void Renderer::BeginScene(Camera &camera, const glm::vec4 &clearColor) {
  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndScene() {}

void Renderer::RenderMesh(Mesh &mesh, Shader &shader, const glm::vec3 &position,
                          const glm::quat &rotation, const glm::vec3 &scale) {}

void Renderer::RenderScene(Scene &scene, Camera &camera, Shader &shader,
                           float tilingFactor, bool renderEditorObjects,
                           float dt, float time, int renderLayer,
                           Camera *cullingCamera, bool useObjCulling,
                           bool useBackfaceCulling,
                           bool useMaterialOptimisation, bool visualizeCulling,
                           bool useAutoLOD, bool useZPrepass,
                           bool useStaticBatching, bool useDynamicBatching) {
  PROFILE_SCOPE("RenderScene_Iterate");
  auto &objects = scene.GetObjects();

  glm::mat4 viewMatrix = camera.GetViewMatrix();
  glm::mat4 projectionMatrix = camera.GetProjectionMatrix();
  glm::mat4 camMatrix = projectionMatrix * viewMatrix;
  glm::vec3 cameraPos = camera.Position;

  shader.use();
  shader.setMat4("view", viewMatrix);
  shader.setMat4("projection", projectionMatrix);
  shader.setMat4("camMatrix", camMatrix);
  shader.setVec3("viewPos", cameraPos);
  shader.setVec3("camPos", cameraPos);
  shader.setFloat("time", time);
  shader.setFloat("iTime", time);
  shader.setFloat("deltaTime", dt);

  if (useBackfaceCulling && renderLayer != 2) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
  } else {
    glDisable(GL_CULL_FACE);
  }

  
  GLint incomingPolyMode[2];
  glGetIntegerv(GL_POLYGON_MODE, incomingPolyMode);
  GLenum basePolyMode = (GLenum)incomingPolyMode[0]; 

  glPolygonMode(GL_FRONT_AND_BACK, basePolyMode);

  Shader *lastShader = &shader;
  bool currentIsWireframe = false;

  Frustum frustum;
  if (useObjCulling || (cullingCamera != nullptr && cullingCamera != &camera &&
                        s_ObjFrustumCulling)) {
    Camera &cRef = cullingCamera ? *cullingCamera : camera;
    frustum = Frustum::CreateFrustumFromCamera(cRef.GetProjectionMatrix() *
                                               cRef.GetViewMatrix());
  }

  for (size_t i = 0; i < objects.size(); ++i) {
    auto &object = objects[i];
    if (!object.isActive)
      continue;
    if (!renderEditorObjects && object.meshType == MeshType::Camera)
      continue;

    glm::mat4 globalTransform = scene.GetGlobalTransform(i);
    glm::mat4 finalMatrix =
        glm::scale(globalTransform, glm::vec3(tilingFactor));

    bool isCulled = false;
    if (useObjCulling || (visualizeCulling && s_ObjFrustumCulling)) {
      glm::vec3 minP = object.mesh.minAABB;
      glm::vec3 maxP = object.mesh.maxAABB;

      glm::vec3 corners[8] = {
          {minP.x, minP.y, minP.z}, {maxP.x, minP.y, minP.z},
          {minP.x, maxP.y, minP.z}, {maxP.x, maxP.y, minP.z},
          {minP.x, minP.y, maxP.z}, {maxP.x, minP.y, maxP.z},
          {minP.x, maxP.y, maxP.z}, {maxP.x, maxP.y, maxP.z}};

      glm::vec3 worldMin(1e30f);
      glm::vec3 worldMax(-1e30f);

      for (int c = 0; c < 8; c++) {
        glm::vec3 worldCorner =
            glm::vec3(finalMatrix * glm::vec4(corners[c], 1.0f));
        worldMin = glm::min(worldMin, worldCorner);
        worldMax = glm::max(worldMax, worldCorner);
      }

      if (!frustum.IsOnFrustum(worldMin, worldMax)) {
        isCulled = true;
      }
    }

    if (isCulled && (!visualizeCulling || !s_ShowCulledAsWireframe)) {
      continue;
    }

    if (renderLayer == 1 && object.material.isTransparent)
      continue;
    if (renderLayer == 2 && !object.material.isTransparent)
      continue;

    Shader *activeShader = &shader;

    
    
    if (useBackfaceCulling && renderLayer != 2) {
      float det = glm::determinant(glm::mat3(globalTransform));
      glFrontFace(det < 0 ? GL_CW : GL_CCW);
    }

    bool shouldBeWireframe = (isCulled && visualizeCulling && renderLayer != 2);

    if (shouldBeWireframe != currentIsWireframe) {
      glPolygonMode(GL_FRONT_AND_BACK,
                    shouldBeWireframe ? GL_LINE : basePolyMode);
      if (shouldBeWireframe) {
        glDisable(GL_CULL_FACE);
      } else if (useBackfaceCulling && renderLayer != 2) {
        glEnable(GL_CULL_FACE);
      }
      currentIsWireframe = shouldBeWireframe;
    }

    if (!object.material.customShaderName.empty()) {
      if (ResourceManager::HasShader(object.material.customShaderName)) {
        activeShader =
            &ResourceManager::GetShader(object.material.customShaderName);
      } else {
        
        std::string name = object.material.customShaderName;
        ResourceManager::LoadShader(name, (name + ".vert").c_str(),
                                    (name + ".frag").c_str());
        if (ResourceManager::HasShader(name)) {
          activeShader = &ResourceManager::GetShader(name);
        }
      }
    }

    if (activeShader != lastShader) {
      activeShader->use();
      activeShader->setMat4("view", viewMatrix);
      activeShader->setMat4("projection", projectionMatrix);
      activeShader->setMat4("camMatrix", camMatrix);
      activeShader->setVec3("viewPos", cameraPos);
      activeShader->setVec3("camPos", cameraPos);
      activeShader->setFloat("time", time);
      activeShader->setFloat("iTime", time);
      activeShader->setFloat("deltaTime", dt);
      lastShader = activeShader;
    }

    
    
    activeShader->setFloat("intensity", object.material.intensity);

    glm::vec3 finalAlbedo = object.material.albedo;
    if (object.hasScreen && (object.screen.type == ScreenType::Image ||
                             object.screen.type == ScreenType::Video)) {
      finalAlbedo *= object.screen.brightness;
    }

    
    
    
    if (!useMaterialOptimisation) {
      activeShader->setVec3("material.albedo", finalAlbedo);
      activeShader->setFloat("material.metallic", object.material.metallic);
      activeShader->setFloat("material.roughness", object.material.roughness);
      activeShader->setFloat("material.ao", object.material.ao);
      activeShader->setFloat("material.shininess", object.material.shininess);
      activeShader->setBool("material.useTexture", object.material.useTexture);
    } else {
      
      activeShader->setVec3("material.albedo", finalAlbedo);
      activeShader->setBool("material.useTexture", object.material.useTexture);
    }

    
    unsigned int texOverride = 0;
    if (object.hasScreen && object.screen.enabled) {
      if (object.screen.type == ScreenType::CameraFeed &&
          object.screen.targetCameraIndex != -1) {
        auto &camObjs = scene.GetObjects();
        if (object.screen.targetCameraIndex < (int)camObjs.size()) {
          texOverride =
              camObjs[object.screen.targetCameraIndex].camera.renderTexture;
        }
      } else if (object.screen.type == ScreenType::Image &&
                 !object.screen.filePath.empty()) {

        if (ResourceManager::HasTexture(object.screen.filePath)) {
          texOverride = ResourceManager::GetTexture(object.screen.filePath).ID;
        } else if (!object.screen.filePath.empty()) {
          ResourceManager::LoadTexture(object.screen.filePath,
                                       object.screen.filePath.c_str(),
                                       "diffuse", 0);
          if (ResourceManager::HasTexture(object.screen.filePath)) {
            texOverride =
                ResourceManager::GetTexture(object.screen.filePath).ID;
          }
        }
      } else if (object.screen.type == ScreenType::Video &&
                 !object.screen.filePath.empty()) {
        if (!object.screen.videoPlayerHandle) {
          VideoPlayer *vp = new VideoPlayer();
          if (vp->Open(object.screen.filePath)) {
            object.screen.videoPlayerHandle = std::shared_ptr<void>(
                vp, [](void *p) { delete static_cast<VideoPlayer *>(p); });
            vp->Update(0.0f);
          } else {
            delete vp;
            object.screen.videoPlayerHandle.reset();
          }

          if (object.screen.videoPlayerHandle) {

            std::string audioPath = object.screen.filePath + "_audio.wav";
            object.hasAudio = true;
            if (object.audio.filePath != audioPath) {
              object.audio.filePath = audioPath;
              object.audio.playing = !object.screen.videoPaused;
              object.audio.playOnAwake = true;
              if (object.audio.playing) {
                AudioEngine::PlayObjectAudio(object);
              }
            }
          }
        }
        if (object.screen.videoPlayerHandle) {
          VideoPlayer *vp =
              static_cast<VideoPlayer *>(object.screen.videoPlayerHandle.get());
          vp->SetLooping(object.screen.videoLoop);

          if (!object.screen.videoPaused) {
            vp->Update(dt * object.screen.videoPlaybackSpeed);
          }
          texOverride = vp->GetTextureID();

          if (object.hasAudio) {
            object.audio.looping = object.screen.videoLoop;
            object.audio.volume = object.screen.videoVolume;
            object.audio.pitch = object.screen.videoPlaybackSpeed;

            if (object.screen.videoPaused && object.audio.playing) {
              object.audio.playing = false;
              AudioEngine::StopObjectAudio(object);
            } else if (!object.screen.videoPaused && !object.audio.playing) {
              object.audio.playing = true;
              AudioEngine::PlayObjectAudio(object);
            }
          }

          finalMatrix = glm::scale(finalMatrix, glm::vec3(1.0f, -1.0f, 1.0f));

          if (object.screen.videoKeepAspect && vp->GetWidth() > 0 &&
              vp->GetHeight() > 0) {
            float aspect = (float)vp->GetWidth() / (float)vp->GetHeight();

            finalMatrix =
                glm::scale(finalMatrix, glm::vec3(aspect, 1.0f, 1.0f));
          }
        }
      }
    }

    if (useBackfaceCulling) {
      
      
      
      
      float det = glm::determinant(glm::mat3(finalMatrix));
      if (det < 0)
        glFrontFace(GL_CW);
      else
        glFrontFace(GL_CCW);
    }

    
    object.mesh.currentLOD = 0;
    if (useAutoLOD && !object.mesh.lodLevels.empty()) {
      float distance = glm::distance(cameraPos, glm::vec3(finalMatrix[3]));
      float maxScale =
          glm::max(glm::length(glm::vec3(finalMatrix[0])),
                   glm::max(glm::length(glm::vec3(finalMatrix[1])),
                            glm::length(glm::vec3(finalMatrix[2]))));
      float radius = glm::length(object.mesh.maxAABB - object.mesh.minAABB) *
                     0.5f * maxScale;

      
      float coverage = radius / glm::max(distance, 0.1f);

      if (coverage < 0.05f && object.mesh.lodLevels.size() >= 3)
        object.mesh.currentLOD = 3;
      else if (coverage < 0.15f && object.mesh.lodLevels.size() >= 2)
        object.mesh.currentLOD = 2;
      else if (coverage < 0.3f && object.mesh.lodLevels.size() >= 1)
        object.mesh.currentLOD = 1;
    }

    object.mesh.Draw(*activeShader, camera, finalMatrix, texOverride);

    if (visualizeCulling && renderLayer != 2) {
      if (!ResourceManager::HasShader("culling_vis")) {
        ResourceManager::LoadShader("culling_vis",
                                    "shaders/editor/culling_vis.vert",
                                    "shaders/editor/culling_vis.frag");
      }

      Shader &debugShader = ResourceManager::GetShader("culling_vis");
      debugShader.use();
      debugShader.setMat4("view", camera.GetViewMatrix());
      debugShader.setMat4("projection", camera.GetProjectionMatrix());
      debugShader.setMat4("model", finalMatrix);
      debugShader.setVec3("cullingCameraPos", cullingCamera->Position);
      debugShader.setBool("isFrustumCulled", isCulled);

      glDisable(GL_CULL_FACE);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(2.0f);
      glEnable(GL_DEPTH_TEST);

      
      int indicesCount = object.mesh.indices.size();
      if (object.mesh.currentLOD != -1 &&
          (size_t)object.mesh.currentLOD < object.mesh.lodLevels.size()) {
        glBindVertexArray(object.mesh.lodLevels[object.mesh.currentLOD].vao);
        indicesCount =
            object.mesh.lodLevels[object.mesh.currentLOD].indices.size();
      } else {
        object.mesh.vao.Bind();
      }

      
      debugShader.setInt("cullingMode", 0);
      glDepthFunc(GL_LEQUAL);
      glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

      
      if (s_BackfaceCulling) {
        debugShader.setInt("cullingMode", 1);
        glDepthFunc(GL_GREATER);
        glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
      }

      glDepthFunc(useZPrepass ? GL_LEQUAL : GL_LESS);
      glLineWidth(1.0f);
      
      if (useBackfaceCulling && renderLayer != 2) {
        glEnable(GL_CULL_FACE);
      }
      glPolygonMode(GL_FRONT_AND_BACK, basePolyMode);
      currentIsWireframe = false;
      activeShader->use();
    }

    if (useBackfaceCulling && renderLayer != 2) {
      glFrontFace(GL_CCW); 
    }
  }

  
  
  if (renderLayer <= 1) {
    if (useStaticBatching && StaticBatcher::HasBatches()) {
      PROFILE_SCOPE("StaticBatching");
      GPU_PROFILE_SCOPE("StaticBatching");
      StaticBatcher::DrawBatches(shader, camera);
    }
    if (useDynamicBatching) {
      PROFILE_SCOPE("DynamicBatching");
      GPU_PROFILE_SCOPE("DynamicBatching");
      DynamicBatcher::DrawBatches(scene, shader, camera);
    }
  }

  if (useBackfaceCulling) {
    glFrontFace(GL_CCW);
  }
}

void Renderer::RenderHitboxes(Scene &scene, Camera &camera) {
  PROFILE_SCOPE("HitboxPass");
  GPU_PROFILE_SCOPE("HitboxPass");

  if (!ResourceManager::HasShader("hitbox")) {

    try {
      ResourceManager::LoadShader("hitbox", "shaders/passes/hitbox.vert",
                                  "shaders/passes/hitbox.frag");
    } catch (...) {
      return;
    }
  }

  Shader &hbShader = ResourceManager::GetShader("hitbox");
  hbShader.use();
  hbShader.setMat4("view", camera.GetViewMatrix());
  hbShader.setMat4("projection", camera.GetProjectionMatrix());
  hbShader.setMat4("camMatrix",
                   camera.GetProjectionMatrix() * camera.GetViewMatrix());

  auto &objects = scene.GetObjects();
  glBindVertexArray(s_boxVAO);
  glLineWidth(2.0f);

  for (size_t i = 0; i < objects.size(); ++i) {
    auto &obj = objects[i];
    if (!obj.isActive)
      continue;

    glm::mat4 model = scene.GetGlobalTransform(i);
    glm::vec3 size = obj.mesh.maxAABB - obj.mesh.minAABB;
    glm::vec3 center = (obj.mesh.maxAABB + obj.mesh.minAABB) * 0.5f;

    glm::mat4 hitboxModel = glm::translate(model, center);
    hitboxModel = glm::scale(hitboxModel, size);

    hbShader.setMat4("model", hitboxModel);

    glm::vec3 color = obj.isStatic ? glm::vec3(1.0f, 1.0f, 0.0f)
                                   : glm::vec3(0.0f, 1.0f, 0.0f);
    hbShader.setVec3("color", color);

    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
  }

  glLineWidth(1.0f);
  glBindVertexArray(0);
}
