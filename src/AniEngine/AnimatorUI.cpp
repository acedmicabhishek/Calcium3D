#include "AnimatorUI.h"
#include "../Core/Logger.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

void AnimatorUI::DrawPanel(Scene& scene, bool& open, const std::set<int>& selectedObjects) {
    if (!open) return;
    
    ImGui::Begin("Animator", &open);
    
    if (selectedObjects.empty()) {
        ImGui::Text("No object selected.");
        ImGui::End();
        return;
    }
    
    int objId = *selectedObjects.begin();
    auto& objects = scene.GetObjects();
    if (objId < 0 || objId >= static_cast<int>(objects.size())) {
        ImGui::Text("Invalid object selected.");
        ImGui::End();
        return;
    }
    
    GameObject& obj = objects[objId];
    ImGui::Text("Animator: %s", obj.name.c_str());
    ImGui::Separator();
    
    ImGui::InputText("##NewAnimName", m_NewAnimName, IM_ARRAYSIZE(m_NewAnimName));
    ImGui::SameLine();
    if (ImGui::Button("Create Clip")) {
        AnimationClip newClip;
        newClip.name = m_NewAnimName;
        newClip.duration = 1.0f;
        newClip.ticksPerSecond = 30.0f;
        obj.animations.push_back(newClip);
        obj.currentAnimationIndex = (int)obj.animations.size() - 1;
    }
    
    if (obj.animations.empty()) {
        ImGui::Text("No animations exist for this object.");
        ImGui::End();
        return;
    }
    
    ImGui::Separator();
    
    ImGui::Checkbox("Play Animation", &obj.isAnimating);
    
    std::vector<const char*> animNames;
    for (const auto& anim : obj.animations) {
        animNames.push_back(anim.name.c_str());
    }
    
    int currentAnim = obj.currentAnimationIndex;
    if (ImGui::Combo("Animation Clip", &currentAnim, animNames.data(), animNames.size())) {
        obj.currentAnimationIndex = currentAnim;
        obj.animationTime = 0.0f;
    }
    
    const auto& activeAnim = obj.animations[obj.currentAnimationIndex];
    float maxTime = activeAnim.duration;
    
    ImGui::Text("Timeline");
    ImGui::SameLine();
    ImGui::Text("(Duration: %.2fs, Ticks/Sec: %.2f)", maxTime, activeAnim.ticksPerSecond);
    
    ImGui::SliderFloat("##Time", &obj.animationTime, 0.0f, maxTime, "%.3fs");
    
    if (ImGui::Button(obj.isAnimating ? "Pause" : "Play ")) {
        obj.isAnimating = !obj.isAnimating;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        obj.isAnimating = false;
        obj.animationTime = 0.0f;
    }
    ImGui::SameLine();
    
    if (m_IsRecording) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    if (ImGui::Button("Rec")) {
        m_IsRecording = !m_IsRecording;
        if (m_IsRecording && obj.isAnimating) obj.isAnimating = false;
    }
    if (m_IsRecording) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Record transforms to keyframes at current time.");
    
    ImGui::End();
}

void AnimatorUI::AddKeyframe(GameObject& obj, int boneIdx, float time) {
    if (obj.currentAnimationIndex < 0 || obj.currentAnimationIndex >= (int)obj.animations.size()) return;
    if (boneIdx < 0 || boneIdx >= (int)obj.mesh.skeleton.bones.size()) return;

    AnimationClip& clip = obj.animations[obj.currentAnimationIndex];
    const auto& bone = obj.mesh.skeleton.bones[boneIdx];
    
    glm::vec3 pos, scale, skew;
    glm::vec4 perspective;
    glm::quat rot;
    glm::decompose(bone.localTransform, scale, rot, pos, skew, perspective);

    AnimationChannel* channel = nullptr;
    for (auto& ch : clip.channels) {
        if (ch.boneName == bone.name) {
            channel = &ch;
            break;
        }
    }
    if (!channel) {
        AnimationChannel newCh;
        newCh.boneName = bone.name;
        clip.channels.push_back(newCh);
        channel = &clip.channels.back();
    }

    auto updateKey = [&](auto& keys, float t, const auto& val) {
        for (auto& k : keys) {
            if (std::abs(k.time - t) < 0.001f) {
                k.value = val;
                return;
            }
        }
        keys.push_back({t, val});
        std::sort(keys.begin(), keys.end(), [](const auto& a, const auto& b) { return a.time < b.time; });
    };

    updateKey(channel->positionKeys, time, pos);
    updateKey(channel->rotationKeys, time, rot);
    updateKey(channel->scaleKeys, time, scale);
    
    if (time > clip.duration) clip.duration = time;
}
