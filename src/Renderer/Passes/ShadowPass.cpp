#include "ShadowPass.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include <glad/glad.h>
#include <iostream>

ShadowPass::ShadowPass()
{
    m_Name = "ShadowPass";
}

ShadowPass::~ShadowPass()
{
    if (m_FBO) glDeleteFramebuffers(1, &m_FBO);
    if (m_DepthMap) glDeleteTextures(1, &m_DepthMap);
}

void ShadowPass::Init()
{
    // Shadow pass is stubbed - don't allocate resources to save memory
    /*
    // Configure Depth Texture
    glGenFramebuffers(1, &m_FBO);
    
    glGenTextures(1, &m_DepthMap);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ShadowWidth, m_ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    ...
    */
}

void ShadowPass::Execute(const RenderContext& context)
{
    // Shadow pass is incomplete (no shader loaded) - skip for now
    // but ensure we restore the correct FBO so subsequent passes render correctly
    glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
    glViewport(0, 0, context.width, context.height);
}

void ShadowPass::Resize(int width, int height)
{
    // Shadow map is fixed resolution usually, independent of viewport
}
