#include "Renderer.h"
#include "AudioEngine/AudioEngine.h"
#include "Core/Logger.h"
#include "Core/ResourceManager.h"
#include "Frustum.h"
#include "VideoPlayer.h"

bool Renderer::s_BackfaceCulling = true;
bool Renderer::s_FrustumCulling = false;

void Renderer::Init() { glEnable(GL_DEPTH_TEST); }

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
                           Camera *cullingCamera) {
  auto &objects = scene.GetObjects();

  shader.use();
  shader.setMat4("view", camera.GetViewMatrix());
  shader.setMat4("projection", camera.GetProjectionMatrix());
  shader.setFloat("time", time);
  shader.setFloat("iTime", time);
  shader.setFloat("deltaTime", dt);

  if (s_BackfaceCulling && renderLayer != 2) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 
  } else {
    glDisable(GL_CULL_FACE);
  }

  Frustum frustum;
  if (s_FrustumCulling) {
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

    bool isCulled = false;
    if (s_FrustumCulling) {
      glm::mat4 model = scene.GetGlobalTransform(i);
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
        glm::vec3 worldCorner = glm::vec3(model * glm::vec4(corners[c], 1.0f));
        worldMin = glm::min(worldMin, worldCorner);
        worldMax = glm::max(worldMax, worldCorner);
      }

      if (!frustum.IsOnFrustum(worldMin, worldMax)) {
        isCulled = true;
      }
    }

    bool isDebugCamera =
        (camera.width > 0 &&
         object.name ==
             "debug_cameraobj"); 
    
    
    
    

    bool visualizingCulling =
        (cullingCamera != nullptr && cullingCamera != &camera);

    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (isCulled && !visualizingCulling) {
      continue;
    }

    if (renderLayer == 1 && object.material.isTransparent)
      continue;
    if (renderLayer == 2 && !object.material.isTransparent)
      continue;

    Shader *activeShader = &shader;

    
    
    bool renderAsWireframe = (isCulled && visualizingCulling);
    if (renderAsWireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (!object.material.customShaderName.empty()) {
      if (!ResourceManager::HasShader(object.material.customShaderName)) {

        std::string sName = object.material.customShaderName;
        std::string vFile = sName + ".vert";
        std::string fFile = sName + ".frag";

        std::string resolvedV = ResourceManager::ResolvePath(vFile);
        if (!std::filesystem::exists(resolvedV)) {
          vFile = "default.vert";
        }

        try {
          ResourceManager::LoadShader(sName, vFile.c_str(), fFile.c_str());
          Logger::AddLog("Auto-loaded custom shader: %s", sName.c_str());
        } catch (...) {
        }
      }

      if (ResourceManager::HasShader(object.material.customShaderName)) {
        activeShader =
            &ResourceManager::GetShader(object.material.customShaderName);
      }
    }
    if (!activeShader) {
      continue;
    }

    activeShader->use();
    activeShader->setMat4("view", camera.GetViewMatrix());
    activeShader->setMat4("projection", camera.GetProjectionMatrix());
    activeShader->setFloat("time", time);
    activeShader->setFloat("iTime", time);
    activeShader->setFloat("deltaTime", dt);
    activeShader->setFloat("intensity", 1.0f);

    glm::vec3 finalAlbedo = object.material.albedo;
    if (object.hasScreen && (object.screen.type == ScreenType::Image ||
                             object.screen.type == ScreenType::Video)) {
      finalAlbedo *= object.screen.brightness;
    }

    activeShader->setVec3("material.albedo", finalAlbedo);
    activeShader->setFloat("material.metallic", object.material.metallic);
    activeShader->setFloat("material.roughness", object.material.roughness);
    activeShader->setFloat("material.ao", object.material.ao);
    activeShader->setFloat("material.shininess", object.material.shininess);
    activeShader->setBool("material.useTexture", object.material.useTexture);

    glm::mat4 globalTransform = scene.GetGlobalTransform(i);

    glm::mat4 finalMatrix =
        glm::scale(globalTransform, glm::vec3(tilingFactor));

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

    if (s_BackfaceCulling && renderLayer != 2) {
      float det = glm::determinant(glm::mat3(finalMatrix));
      if (det < 0)
        glFrontFace(GL_CW);
      else
        glFrontFace(GL_CCW);
    }

    object.mesh.Draw(*activeShader, camera, finalMatrix, texOverride);

    
    
    if (visualizingCulling && s_BackfaceCulling && renderLayer != 2) {
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
      debugShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
      debugShader.setVec3("cullingCameraPos", cullingCamera->Position);
      debugShader.setBool("isBackfacePass", true);

      glDisable(GL_CULL_FACE); 
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDisable(GL_DEPTH_TEST);
      glLineWidth(2.0f);

      object.mesh.vao.Bind();
      glDrawElements(GL_TRIANGLES, object.mesh.indices.size(), GL_UNSIGNED_INT,
                     0);

      
      glLineWidth(1.0f);
      glEnable(GL_DEPTH_TEST);
      if (s_BackfaceCulling && renderLayer != 2) {
        glEnable(GL_CULL_FACE);
      }
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      activeShader->use();
    }

    
    if (s_BackfaceCulling && renderLayer != 2) {
      glFrontFace(GL_CCW);
    }
  }
}

void Renderer::RenderHitboxes(Scene &scene, Camera &camera) {
  if (!ResourceManager::HasShader("hitbox")) {

    try {
      ResourceManager::LoadShader("hitbox", "shaders/passes/hitbox.vert",
                                  "shaders/passes/hitbox.frag");
    } catch (...) {
      return;
    }
  }

  static unsigned int boxVAO = 0, boxVBO = 0, boxEBO = 0;
  if (boxVAO == 0) {
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, 0.5f,
                        -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f, 0.5f};
    unsigned int indices[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                              6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glGenBuffers(1, &boxEBO);

    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }

  Shader &hbShader = ResourceManager::GetShader("hitbox");
  hbShader.use();
  hbShader.setMat4("view", camera.GetViewMatrix());
  hbShader.setMat4("projection", camera.GetProjectionMatrix());

  auto &objects = scene.GetObjects();
  glBindVertexArray(boxVAO);
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
