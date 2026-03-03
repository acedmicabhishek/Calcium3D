#ifndef RIGGING_UI_H
#define RIGGING_UI_H

#include "../Scene/Scene.h"
#include "../Renderer/Camera.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <functional>

class RiggingUI {
public:
    void RenderBoneOverlay(Scene& scene, Camera& camera, int selectedCube, ImVec2 cursorPos, float viewportWidth, float viewportHeight);
    void DrawRiggingControls();
    void DrawBoneHierarchy(Scene& scene, int objIndex, int& selectedCube);
    void DrawRiggingInspector(GameObject& obj, Scene& scene);

    static glm::mat4 GetBoneGlobalTransform(const GameObject& obj, int boneIdx, const Scene& scene, int objIdx);

    bool editRigMode = false;
    int selectedBone = -1;
};

#endif
