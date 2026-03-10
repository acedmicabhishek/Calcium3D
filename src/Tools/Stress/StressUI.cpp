#include "StressUI.h"
#ifndef C3D_RUNTIME
#include "../../Scene/ObjectFactory.h"
#include <imgui.h>
#include <random>

void StressUI::Draw(bool *pOpen, Scene &scene) {
  if (!ImGui::Begin("Stress Test", pOpen)) {
    ImGui::End();
    return;
  }

  static int gridX = 10;
  static int gridY = 1;
  static int gridZ = 10;
  static float spacing = 2.0f;

  if (ImGui::CollapsingHeader("Grid Spawner", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputInt("Rows (X)", &gridX);
    ImGui::InputInt("Layers (Y)", &gridY);
    ImGui::InputInt("Cols (Z)", &gridZ);
    ImGui::DragFloat("Spacing", &spacing, 0.1f, 0.1f, 10.0f);

    if (ImGui::Button("Spawn Cube Grid")) {
      for (int x = 0; x < gridX; x++) {
        for (int y = 0; y < gridY; y++) {
          for (int z = 0; z < gridZ; z++) {
            GameObject obj(ObjectFactory::createCube(), "Stress_Cube");
            obj.position = glm::vec3(x * spacing, y * spacing, z * spacing);
            obj.meshType = MeshType::Cube;
            scene.AddObject(std::move(obj));
          }
        }
      }
    }
  }

  ImGui::Separator();

  static int randomCount = 100;
  static float randomRadius = 50.0f;

  if (ImGui::CollapsingHeader("Random Spawner",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputInt("Count", &randomCount);
    ImGui::DragFloat("Radius", &randomRadius, 1.0f, 1.0f, 500.0f);

    if (ImGui::Button("Spawn Random Cubes")) {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float> dis(-randomRadius, randomRadius);

      for (int i = 0; i < randomCount; i++) {
        GameObject obj(ObjectFactory::createCube(), "Stress_Cube_Random");
        obj.position = glm::vec3(dis(gen), dis(gen), dis(gen));
        obj.meshType = MeshType::Cube;
        scene.AddObject(std::move(obj));
      }
    }
  }

  ImGui::Separator();

  static int lightCount = 50;
  static float lightRadius = 30.0f;

  if (ImGui::CollapsingHeader("Light Spawner",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputInt("Light Count", &lightCount);
    ImGui::DragFloat("Light Radius", &lightRadius, 1.0f, 1.0f, 200.0f);

    if (ImGui::Button("Spawn Point Lights")) {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float> posDis(-lightRadius, lightRadius);
      std::uniform_real_distribution<float> colDis(0.5f, 1.0f);

      for (int i = 0; i < lightCount; i++) {
        Scene::PointLight *pl = scene.CreatePointLight();
        if (pl) {
          pl->position =
              glm::vec3(posDis(gen), posDis(gen) + 5.0f, posDis(gen));
          pl->color = glm::vec4(colDis(gen), colDis(gen), colDis(gen), 1.0f);
          pl->intensity = 1.0f;
        }
      }
    }
  }

  ImGui::Separator();

  float halfWidth = ImGui::GetContentRegionAvail().x * 0.5f - 4.0f;

  if (ImGui::Button("Clear Stress Objects", ImVec2(halfWidth, 0))) {
    auto &objects = scene.GetObjects();
    for (int i = (int)objects.size() - 1; i >= 0; i--) {
      if (objects[i].name.find("Stress_") == 0) {
        scene.RemoveObject(i);
      }
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear ALL Lights", ImVec2(-FLT_MIN, 0))) {
    scene.GetPointLights().clear();
  }

  if (ImGui::Button("Clear ALL Stress Content", ImVec2(-FLT_MIN, 0))) {
    auto &objects = scene.GetObjects();
    for (int i = (int)objects.size() - 1; i >= 0; i--) {
      if (objects[i].name.find("Stress_") == 0) {
        scene.RemoveObject(i);
      }
    }
    scene.GetPointLights().clear();
  }

  ImGui::End();
}
#endif
