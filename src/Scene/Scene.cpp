#include "Scene.h"
#include "../Core/ThreadManager.h"
#include "../Tools/Profiler/Profiler.h"
#include "SceneManager.h"
#include <algorithm>
#include <unordered_map>

Scene::Scene() {}

Scene::~Scene() { Clear(); }

void Scene::Update(float dt, float time) {
  {
    PROFILE_SCOPE("Physics");
    physicsEngine.Update(dt, time, m_Objects);
  }

  {
    PROFILE_SCOPE("Billboarding");
    for (auto &obj : m_Objects) {
      if (obj.is2DSprite && obj.sprite.faceCamera) {
        glm::vec3 targetPos = glm::vec3(0.0f);
        bool validTarget = false;

        if (obj.sprite.targetCameraIndex == -1) {
          if (auto mainCam = SceneManager::Get().GetMainCamera()) {
            targetPos = mainCam->Position;
            validTarget = true;
          }
        } else if (obj.sprite.targetCameraIndex >= 0 &&
                   obj.sprite.targetCameraIndex < m_Objects.size()) {
          if (m_Objects[obj.sprite.targetCameraIndex].hasCamera) {
            targetPos = m_Objects[obj.sprite.targetCameraIndex].position;
            validTarget = true;
          }
        }

        if (validTarget) {
          glm::vec3 direction = targetPos - obj.position;
          direction.y = 0.0f;
          if (glm::length(direction) > 0.001f) {
            direction = glm::normalize(direction);
            glm::quat lookAtRot =
                glm::quatLookAt(-direction, glm::vec3(0.0f, 1.0f, 0.0f));
            
            
            
            obj.rotation = lookAtRot * glm::quat(glm::vec3(glm::radians(90.0f),
                                                           0.0f, 0.0f));
          }
        }
      }
    }
  }

  if (ThreadManager::IsEnabled()) {
    PROFILE_SCOPE("Scripts");
    ThreadManager::ParallelFor(0, (int)m_Objects.size(), [&](int i) {
      auto &obj = m_Objects[i];

      for (auto &script : obj.behaviors) {
        if (script) {
          script->gameObject = &obj;
          if (script->enabled)
            script->OnUpdate(dt);
        }
      }
    });
  } else {
    PROFILE_SCOPE("Scripts");
    for (auto &obj : m_Objects) {
      for (auto &script : obj.behaviors) {
        if (script) {
          script->gameObject = &obj;
          if (script->enabled)
            script->OnUpdate(dt);
        }
      }
    }
  }
}

void Scene::AddObject(GameObject object) {
  m_Objects.push_back(std::move(object));
}

void Scene::RemoveObject(int index) {
  if (index >= 0 && index < (int)m_Objects.size()) {
    m_Objects.erase(m_Objects.begin() + index);

    for (auto &obj : m_Objects) {
      if (obj.parentIndex == index) {
        obj.parentIndex = -1;
      } else if (obj.parentIndex > index) {
        obj.parentIndex--;
      }
    }

    if (auto mainCam = SceneManager::Get().GetMainCamera()) {
      if (mainCam->parentIndex == index) {
        mainCam->parentIndex = -1;
      } else if (mainCam->parentIndex > index) {
        mainCam->parentIndex--;
      }
    }
  }
}

void Scene::RemoveObjectTree(int index) {
  if (index < 0 || index >= m_Objects.size())
    return;

  std::vector<int> toDelete;
  toDelete.push_back(index);

  for (size_t i = 0; i < toDelete.size(); ++i) {
    int curr = toDelete[i];
    for (int j = 0; j < m_Objects.size(); ++j) {
      if (m_Objects[j].parentIndex == curr) {
        toDelete.push_back(j);
      }
    }
  }

  std::sort(toDelete.begin(), toDelete.end(), std::greater<int>());

  for (int idx : toDelete) {
    RemoveObject(idx);
  }
}

void Scene::DuplicateObjectTree(int index, int newParentIndex) {
  if (index < 0 || index >= m_Objects.size())
    return;

  std::unordered_map<int, int> indexMap;
  std::vector<int> toCopy;
  toCopy.push_back(index);

  for (size_t i = 0; i < toCopy.size(); ++i) {
    int curr = toCopy[i];
    for (int j = 0; j < m_Objects.size(); ++j) {
      if (m_Objects[j].parentIndex == curr) {
        toCopy.push_back(j);
      }
    }
  }

  for (int oldIdx : toCopy) {
    GameObject &src = m_Objects[oldIdx];

    Mesh newMesh(const_cast<std::vector<Vertex> &>(src.mesh.vertices),
                 const_cast<std::vector<GLuint> &>(src.mesh.indices),
                 src.mesh.textures);

    GameObject newObj(std::move(newMesh),
                      src.name + (oldIdx == index ? " (Copy)" : ""));
    newObj.position = src.position;
    newObj.rotation = src.rotation;
    newObj.scale = src.scale;
    newObj.material = src.material;
    newObj.shape = src.shape;
    newObj.collisionRadius = src.collisionRadius;
    newObj.useGravity = src.useGravity;
    newObj.isStatic = src.isStatic;
    newObj.mass = src.mass;
    newObj.friction = src.friction;
    newObj.restitution = src.restitution;
    newObj.enableCollision = src.enableCollision;
    newObj.centerOfMassOffset = src.centerOfMassOffset;
    newObj.isFolded = src.isFolded;

    newObj.hasCamera = src.hasCamera;
    newObj.camera = src.camera;
    newObj.hasScreen = src.hasScreen;
    newObj.screen = src.screen;
    newObj.hasWater = src.hasWater;
    newObj.water = src.water;
    newObj.hasSDF = src.hasSDF;
    newObj.sdf = src.sdf;
    newObj.is2DSprite = src.is2DSprite;
    newObj.sprite = src.sprite;

    if (oldIdx == index) {
      newObj.parentIndex = newParentIndex;
    } else {
      newObj.parentIndex = indexMap[src.parentIndex];
    }

    m_Objects.push_back(std::move(newObj));
    int newIdx = m_Objects.size() - 1;
    indexMap[oldIdx] = newIdx;
  }
}

void Scene::Clear() {
  m_Objects.clear();
  m_PointLights.clear();
  m_Flags.clear();
  m_Filepath = "";
}

Scene::PointLight *Scene::CreatePointLight() {
  PointLight pl;

  pl.position = glm::vec3(0.0f, 5.0f, 0.0f);
  pl.color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
  pl.intensity = 5.0f;
  pl.enabled = true;
  pl.constant = 1.0f;
  pl.linear = 0.09f;
  pl.quadratic = 0.032f;
  pl.castShadows = false;

  m_PointLights.push_back(pl);
  return &m_PointLights.back();
}

void Scene::RemovePointLight(int index) {
  if (index >= 0 && index < (int)m_PointLights.size()) {
    m_PointLights.erase(m_PointLights.begin() + index);
  }
}

glm::mat4 Scene::GetGlobalTransform(int objectIndex) const {
  if (objectIndex < 0 || objectIndex >= m_Objects.size()) {
    return glm::mat4(1.0f);
  }

  const GameObject &obj = m_Objects[objectIndex];
  glm::mat4 local = glm::translate(glm::mat4(1.0f), obj.position) *
                    glm::mat4_cast(obj.rotation) *
                    glm::scale(glm::mat4(1.0f), obj.scale);

  if (obj.parentIndex != -1 && obj.parentIndex != objectIndex &&
      obj.parentIndex < m_Objects.size()) {
    static int depthCount = 0;
    if (depthCount > 200) {
      depthCount = 0;
      return local;
    }
    depthCount++;
    glm::mat4 result = GetGlobalTransform(obj.parentIndex) * local;
    depthCount--;
    return result;
  }

  return local;
}

void Scene::AddFlag(const std::string &name, const glm::vec3 &position,
                    float yaw, float pitch) {
  m_Flags[name] = {position, yaw, pitch};
}

void Scene::RemoveFlag(const std::string &name) { m_Flags.erase(name); }

Scene::FlagData Scene::GetFlag(const std::string &name) const {
  auto it = m_Flags.find(name);
  if (it != m_Flags.end())
    return it->second;
  return {};
}
