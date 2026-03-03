#ifndef HITBOXGRAPHICS_H
#define HITBOXGRAPHICS_H

#include "../Renderer/Shader.h"
#include "../Renderer/Camera.h"
#include "../Scene/Scene.h"
#include "PhysicsEngine.h"

class HitboxGraphics {
public:
    static void Init();
    static void Render(Scene& scene, Camera& camera);
    static void Cleanup();

    static bool ShowHitboxes;

private:
    static unsigned int m_VAO, m_VBO, m_EBO;
    static unsigned int m_DynVAO, m_DynVBO;
};

#endif
