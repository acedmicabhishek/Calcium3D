#ifndef ANIMATOR_UI_H
#define ANIMATOR_UI_H

#include "../Scene/Scene.h"
#include <imgui.h>
#include <set>

class AnimatorUI {
public:
    void DrawPanel(Scene& scene, bool& open, const std::set<int>& selectedObjects);
    void AddKeyframe(GameObject& obj, int boneIdx, float time);
    
    bool IsRecording() const { return m_IsRecording; }
    void SetRecording(bool recording) { m_IsRecording = recording; }

private:
    bool m_IsRecording = false;
    char m_NewAnimName[64] = "NewAnimation";
};

#endif
