#include "RiggingUI.h"
#include "../Core/Logger.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

void RiggingUI::RenderBoneOverlay(Scene& scene, Camera& camera, int selectedCube, ImVec2 cursorPos, float viewportWidth, float viewportHeight) {
    if (!editRigMode || selectedCube < 0) return;
    auto& objects = scene.GetObjects();
    if (selectedCube >= (int)objects.size()) return;
    
    auto& obj = objects[selectedCube];
    if (obj.mesh.skeleton.bones.empty()) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    for (int i = 0; i < (int)obj.mesh.skeleton.bones.size(); ++i) {
        glm::mat4 boneWorld = GetBoneGlobalTransform(obj, i, scene, selectedCube);
        glm::vec3 bonePos(boneWorld[3]);
        
        glm::vec4 clip = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(bonePos, 1.0f);
        if (clip.w > 0.0f) {
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            float screenX = cursorPos.x + (ndc.x + 1.0f) * 0.5f * viewportWidth;
            float screenY = cursorPos.y + (1.0f - ndc.y) * 0.5f * viewportHeight;
            
            int parentIdx = obj.mesh.skeleton.bones[i].parentIndex;
            if (parentIdx != -1 && parentIdx < (int)obj.mesh.skeleton.bones.size()) {
                glm::mat4 parentWorld = GetBoneGlobalTransform(obj, parentIdx, scene, selectedCube);
                glm::vec3 parentPos(parentWorld[3]);
                glm::vec4 pClip = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(parentPos, 1.0f);
                if (pClip.w > 0.0f) {
                    glm::vec3 pNdc = glm::vec3(pClip) / pClip.w;
                    float pScreenX = cursorPos.x + (pNdc.x + 1.0f) * 0.5f * viewportWidth;
                    float pScreenY = cursorPos.y + (1.0f - pNdc.y) * 0.5f * viewportHeight;
                    drawList->AddLine(ImVec2(pScreenX, pScreenY), ImVec2(screenX, screenY), IM_COL32(255, 255, 0, 255), 2.0f);
                }
            }
            
            ImU32 col = (selectedBone == i) ? IM_COL32(255, 100, 100, 255) : IM_COL32(100, 255, 100, 255);
            drawList->AddCircleFilled(ImVec2(screenX, screenY), (selectedBone == i) ? 6.0f : 4.0f, col);
        }
    }
}

void RiggingUI::DrawRiggingControls() {
    if (ImGui::Checkbox("Rigging Mode", &editRigMode)) {
        if (!editRigMode) selectedBone = -1;
    }
}

void RiggingUI::DrawBoneHierarchy(Scene& scene, int objIndex, int& selectedCube) {
    if (!editRigMode) return;
    auto& objects = scene.GetObjects();
    if (objIndex < 0 || objIndex >= (int)objects.size()) return;
    auto& obj = objects[objIndex];
    if (obj.mesh.skeleton.bones.empty()) return;

    if (ImGui::TreeNodeEx((void*)(intptr_t)(objIndex + 500000), ImGuiTreeNodeFlags_DefaultOpen, "Skeleton")) {
        std::function<void(int, int)> drawBone = [&](int bIdx, int objIdx) {
            auto& bone = objects[objIdx].mesh.skeleton.bones[bIdx];
            ImGuiTreeNodeFlags bFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (selectedBone == bIdx && selectedCube == objIdx) bFlags |= ImGuiTreeNodeFlags_Selected;
            
            bool bHasChildren = false;
            for (size_t bi = 0; bi < objects[objIdx].mesh.skeleton.bones.size(); ++bi) {
                if (objects[objIdx].mesh.skeleton.bones[bi].parentIndex == bIdx) { bHasChildren = true; break;}
            }
            if (!bHasChildren) bFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            
            bool bOpen = ImGui::TreeNodeEx((void*)(intptr_t)(bIdx + 1000000 * (objIdx + 1)), bFlags, "%s", bone.name.c_str());
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                selectedCube = objIdx;
                selectedBone = (int)bIdx;
            }
            
            if (bOpen) {
                if (bHasChildren) {
                    for (size_t bi = 0; bi < objects[objIdx].mesh.skeleton.bones.size(); ++bi) {
                        if (objects[objIdx].mesh.skeleton.bones[bi].parentIndex == bIdx) {
                            drawBone((int)bi, objIdx);
                        }
                    }
                }
                if (bHasChildren) ImGui::TreePop();
            }
            
            if (ImGui::BeginPopupContextItem((std::string("BoneContext") + std::to_string(bIdx)).c_str())) {
                if (ImGui::MenuItem("Add Child Bone")) {
                    Bone newBone;
                    newBone.name = "Bone_" + std::to_string(objects[objIdx].mesh.skeleton.bones.size());
                    newBone.index = objects[objIdx].mesh.skeleton.bones.size();
                    newBone.parentIndex = bIdx;
                    newBone.localTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.0f, 0));
                    objects[objIdx].mesh.skeleton.bones.push_back(newBone);
                }
                ImGui::EndPopup();
            }
        };
        
        if (ImGui::BeginPopupContextItem("SkeletonContext")) {
            if (ImGui::MenuItem("Add Root Bone")) {
                Bone newBone;
                newBone.name = "RootBone_" + std::to_string(objects[objIndex].mesh.skeleton.bones.size());
                newBone.index = objects[objIndex].mesh.skeleton.bones.size();
                newBone.parentIndex = -1;
                newBone.localTransform = glm::mat4(1.0f);
                objects[objIndex].mesh.skeleton.bones.push_back(newBone);
            }
            ImGui::EndPopup();
        }
        
        for (size_t bi = 0; bi < objects[objIndex].mesh.skeleton.bones.size(); ++bi) {
            if (objects[objIndex].mesh.skeleton.bones[bi].parentIndex == -1) {
                drawBone((int)bi, objIndex);
            }
        }
        ImGui::TreePop();
    }
}

void RiggingUI::DrawRiggingInspector(GameObject& obj, Scene& scene) {
    if (obj.meshType == MeshType::Model) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Rigging & Animation");
        
        if (ImGui::Button("Bind Skin to Rig")) {
            Logger::AddLog("Binding skin for %s...", obj.name.c_str());
            auto& skeleton = obj.mesh.skeleton;
            if (skeleton.bones.empty()) {
                Logger::AddLog("[ERROR] No bones found in skeleton!");
            } else {
                for (int i = 0; i < (int)skeleton.bones.size(); ++i) {
                    skeleton.bones[i].offsetMatrix = glm::inverse(GetBoneGlobalTransform(obj, i, scene, -1));
                }

                for (auto& vertex : obj.mesh.vertices) {
                    struct BoneDist { int id; float dist; };
                    std::vector<BoneDist> dists;
                    for (int i = 0; i < (int)skeleton.bones.size(); ++i) {
                        glm::mat4 boneGlobal = GetBoneGlobalTransform(obj, i, scene, -1);
                        glm::vec3 bonePos(boneGlobal[3]);
                        float d = glm::distance(vertex.position, bonePos);
                        dists.push_back({i, d});
                    }
                    std::sort(dists.begin(), dists.end(), [](const BoneDist& a, const BoneDist& b) { return a.dist < b.dist; });

                    float totalInvDist = 0.0f;
                    int count = std::min((int)dists.size(), 4);
                    for (int i = 0; i < 4; ++i) { vertex.boneIds[i] = -1; vertex.weights[i] = 0.0f; }
                    for (int i = 0; i < count; ++i) {
                        vertex.boneIds[i] = dists[i].id;
                        float w = 1.0f / (dists[i].dist + 0.0001f);
                        vertex.weights[i] = w;
                        totalInvDist += w;
                    }
                    if (totalInvDist > 0.0f) { for (int i = 0; i < count; ++i) vertex.weights[i] /= totalInvDist; }
                }
                obj.mesh.UpdateVBO();
                Logger::AddLog("Skin bound successfully to %zu bones", skeleton.bones.size());
            }
        }

        if (ImGui::Button("Clear Skeleton")) {
            obj.mesh.skeleton.bones.clear();
            obj.mesh.skeleton.boneMapping.clear();
            Logger::AddLog("Skeleton cleared for %s", obj.name.c_str());
        }
    }
}

glm::mat4 RiggingUI::GetBoneGlobalTransform(const GameObject& obj, int boneIdx, const Scene& scene, int objIdx) {
    if (boneIdx < 0 || boneIdx >= (int)obj.mesh.skeleton.bones.size()) {
        if (objIdx >= 0) return scene.GetGlobalTransform(objIdx);
        return glm::mat4(1.0f);
    }
    glm::mat4 pMat = GetBoneGlobalTransform(obj, obj.mesh.skeleton.bones[boneIdx].parentIndex, scene, objIdx);
    return pMat * obj.mesh.skeleton.bones[boneIdx].localTransform;
}
