#include "SSRPass.h"
#include "../../Core/ResourceManager.h"
#include "../../Environment/2dCloud.h"
#include "../../Scene/Scene.h"
#include "../Camera.h"
#include "../Frustum.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "SkyPass.h"
#include <glad/glad.h>

SSRPass::SSRPass(PassType type) : m_PassType(type) {}

unsigned int SSRPass::s_GeometryNormalArchive = 0;
unsigned int SSRPass::s_GeometryDepthArchive = 0;
unsigned int SSRPass::s_ArchiveFBO = 0;
int SSRPass::s_ArchiveWidth = 0;
int SSRPass::s_ArchiveHeight = 0;

void SSRPass::EnsureGeoNormalArchive(int w, int h) {

  if (s_GeometryNormalArchive != 0 && s_ArchiveWidth == w &&
      s_ArchiveHeight == h)
    return;
  if (s_GeometryNormalArchive) {
    glDeleteTextures(1, &s_GeometryNormalArchive);
    s_GeometryNormalArchive = 0;
  }
  if (s_GeometryDepthArchive) {
    glDeleteTextures(1, &s_GeometryDepthArchive);
    s_GeometryDepthArchive = 0;
  }
  if (s_ArchiveFBO) {
    glDeleteFramebuffers(1, &s_ArchiveFBO);
    s_ArchiveFBO = 0;
  }
  s_ArchiveWidth = w;
  s_ArchiveHeight = h;

  glGenTextures(1, &s_GeometryNormalArchive);
  glBindTexture(GL_TEXTURE_2D, s_GeometryNormalArchive);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenTextures(1, &s_GeometryDepthArchive);
  glBindTexture(GL_TEXTURE_2D, s_GeometryDepthArchive);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0, GL_DEPTH_STENCIL,
               GL_UNSIGNED_INT_24_8, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(1, &s_ArchiveFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, s_ArchiveFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         s_GeometryNormalArchive, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, s_GeometryDepthArchive, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

SSRPass::~SSRPass() {
  if (m_ResolveFBO != 0) {
    glDeleteFramebuffers(1, &m_ResolveFBO);
    glDeleteTextures(1, &m_ResolveColorObj);
    glDeleteTextures(1, &m_ResolveNormalObj);
    glDeleteTextures(1, &m_ResolveDepthObj);
  }
  if (m_QuadVAO != 0) {
    glDeleteVertexArrays(1, &m_QuadVAO);
    glDeleteBuffers(1, &m_QuadVBO);
  }
}

void SSRPass::Init() {
  SetupQuad();
  if (!ResourceManager::HasShader("ssr")) {
    ResourceManager::LoadShader("ssr", "../shaders/passes/postprocess/ssr.vert",
                                "../shaders/passes/postprocess/ssr.frag");
  }
  m_SSRShader = &ResourceManager::GetShader("ssr");
}

void SSRPass::ResizeResolveFBO(int width, int height) {
  if (width == 0 || height == 0)
    return;

  if (m_ResolveFBO != 0) {
    glDeleteFramebuffers(1, &m_ResolveFBO);
    glDeleteTextures(1, &m_ResolveColorObj);
    glDeleteTextures(1, &m_ResolveNormalObj);
    glDeleteTextures(1, &m_ResolveDepthObj);
  }

  m_LastWidth = width;
  m_LastHeight = height;

  glGenFramebuffers(1, &m_ResolveFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_ResolveFBO);

  glGenTextures(1, &m_ResolveColorObj);
  glBindTexture(GL_TEXTURE_2D, m_ResolveColorObj);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_ResolveColorObj, 0);

  glGenTextures(1, &m_ResolveNormalObj);
  glBindTexture(GL_TEXTURE_2D, m_ResolveNormalObj);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         m_ResolveNormalObj, 0);

  glGenTextures(1, &m_ResolveDepthObj);
  glBindTexture(GL_TEXTURE_2D, m_ResolveDepthObj);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,
               GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, m_ResolveDepthObj, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSRPass::Execute(const RenderContext &context) {
  if (context.width <= 0 || context.height <= 0 || context.mainFBO == 0)
    return;

  PROFILE_SCOPE("SSRPass");
  GPU_PROFILE_SCOPE("SSRPass");

  bool isSSR = (context.reflectionMode == 1);
  bool isCubemap = (context.reflectionMode == 2);

  if (m_PassType == PassType::CubemapOnly) {
    if (!isCubemap)
      return;
  } else {
    if (!isSSR)
      return;
    if (m_PassType == PassType::Geometry && !context.ssrGeometry)
      return;
    if (m_PassType == PassType::Transparency && !context.ssrTransparency)
      return;
    if (m_PassType == PassType::All && !context.ssrAll)
      return;
  }

  
  if (context.scene && context.camera) {
    bool hasReflectiveObjects = false;
    Frustum camFrustum =
        Frustum::CreateFrustumFromCamera(context.camera->GetProjectionMatrix() *
                                         context.camera->GetViewMatrix());

    auto &objects = context.scene->GetObjects();
    for (size_t i = 0; i < objects.size(); ++i) {
      const auto &obj = objects[i];
      if (!obj.isActive || obj.meshType == MeshType::Camera)
        continue;

      
      bool isReflective = (obj.material.metallic > 0.05f) ||
                          (obj.material.roughness < 0.95f) || (obj.hasWater);
      if (!isReflective)
        continue;

      glm::mat4 model = context.scene->GetGlobalTransform(i);
      glm::vec3 center = glm::vec3(
          model *
          glm::vec4((obj.mesh.minAABB + obj.mesh.maxAABB) * 0.5f, 1.0f));
      float radius = glm::length(obj.mesh.maxAABB - obj.mesh.minAABB) * 0.5f;
      float maxScale = glm::max(glm::length(glm::vec3(model[0])),
                                glm::max(glm::length(glm::vec3(model[1])),
                                         glm::length(glm::vec3(model[2]))));
      radius *= maxScale;

      
      if (camFrustum.IsSphereOnFrustum(center, radius)) {
        float distToCam = glm::distance(context.camera->Position, center);
        if (distToCam - radius < context.ssrRenderDistance) {
          hasReflectiveObjects = true;
          break; 
        }
      }
    }

    if (!hasReflectiveObjects)
      return; 
  }

  if (context.width != m_LastWidth || context.height != m_LastHeight) {
    ResizeResolveFBO(context.width, context.height);
  }

  PROFILE_SCOPE("SSR_Blit");
  glBindFramebuffer(GL_READ_FRAMEBUFFER, context.mainFBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolveFBO);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                    context.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                    context.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                    context.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, attachments);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (!m_SSRShader) {
    if (!ResourceManager::HasShader("ssr")) {
      ResourceManager::LoadShader("ssr",
                                  "../shaders/passes/postprocess/ssr.vert",
                                  "../shaders/passes/postprocess/ssr.frag");
    }
    m_SSRShader = &ResourceManager::GetShader("ssr");
  }
  m_SSRShader->use();

  
  
  glm::mat4 projection = context.camera->GetProjectionMatrix();
  glm::mat4 view = context.camera->GetViewMatrix();
  m_SSRShader->setMat4("projection", projection);
  m_SSRShader->setMat4("view", view);
  m_SSRShader->setMat4("invProjection", glm::inverse(projection));
  m_SSRShader->setMat4("invView", glm::inverse(view));
  m_SSRShader->setVec3("camPos", context.camera->Position);

  m_SSRShader->setFloat("ssrResolution", context.ssrResolution);
  m_SSRShader->setInt("ssrMaxSteps", context.ssrMaxSteps);
  m_SSRShader->setFloat("ssrMaxDistance", context.ssrMaxDistance);
  m_SSRShader->setFloat("ssrThickness", context.ssrThickness);
  m_SSRShader->setFloat("ssrRenderDistance", context.ssrRenderDistance);
  m_SSRShader->setFloat("ssrFadeStart", context.ssrFadeStart);
  m_SSRShader->setInt("ssrUseCubemapFallback",
                      context.ssrUseCubemapFallback ? 1 : 0);
  m_SSRShader->setInt("reflectionMode", context.reflectionMode);

  m_SSRShader->setInt("isAllPass", (m_PassType == PassType::All) ? 1 : 0);

  m_SSRShader->setInt("u_showClouds",
                      (context.showClouds && context.cloudMode == 0) ? 1 : 0);
  m_SSRShader->setFloat("u_time", context.time);
  m_SSRShader->setFloat("u_cloudHeight", context.cloudHeight);

  glm::vec3 cloudColor(1.0f);
  float cloudCover = context.cloudCover;
  float cloudSpeed = 0.1f;
  float tiling = 1.0f;
  float density = context.cloudDensity;
  float cloudSize = 2.0f;
  float randomness = 0.5f;

  if (context.cloud2d) {
    cloudColor = context.cloud2d->cloudColor;
    cloudCover = context.cloud2d->cloudCover;
    cloudSpeed = context.cloud2d->cloudSpeed;
    tiling = context.cloud2d->tiling;
    density = context.cloud2d->density;
    cloudSize = context.cloud2d->cloudSize;
    randomness = context.cloud2d->randomness;
  }

  m_SSRShader->setVec3("u_cloudColor", cloudColor);
  m_SSRShader->setFloat("u_cloudCover", cloudCover);
  m_SSRShader->setFloat("u_cloudSpeed", cloudSpeed);
  m_SSRShader->setFloat("u_tiling", tiling);
  m_SSRShader->setFloat("u_density", density);
  m_SSRShader->setFloat("u_cloudSize", cloudSize);
  m_SSRShader->setFloat("u_randomness", randomness);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_ResolveColorObj);
  m_SSRShader->setInt("colorTexture", 0);

  glActiveTexture(GL_TEXTURE1);

  if (m_PassType == PassType::Geometry) {

    EnsureGeoNormalArchive(context.width, context.height);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ResolveFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_ArchiveFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                      context.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                      context.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
    unsigned int drawBufs[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBufs);
    glBindTexture(GL_TEXTURE_2D, m_ResolveNormalObj);
  } else if (m_PassType == PassType::All && s_GeometryNormalArchive != 0) {
    glBindTexture(GL_TEXTURE_2D, s_GeometryNormalArchive);
  } else {
    glBindTexture(GL_TEXTURE_2D, m_ResolveNormalObj);
  }
  m_SSRShader->setInt("normalTexture", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_ResolveDepthObj);
  m_SSRShader->setInt("depthTexture", 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_CUBE_MAP, SkyPass::GetCubemapTexture());
  m_SSRShader->setInt("skyboxCubemap", 3);

  PROFILE_SCOPE("SSR_Draw");
  glBindVertexArray(m_QuadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  glDrawBuffers(2, attachments);
}

void SSRPass::SetupQuad() {
  float quadVertices[] = {

      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };

  float finalQuad[] = {-1.0f, 1.0f, 0.0f, 0.0f,  1.0f, -1.0f, -1.0f, 0.0f,
                       0.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  0.0f,

                       -1.0f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  -1.0f, 0.0f,
                       1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f};

  glGenVertexArrays(1, &m_QuadVAO);
  glGenBuffers(1, &m_QuadVBO);
  glBindVertexArray(m_QuadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(finalQuad), &finalQuad, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}
