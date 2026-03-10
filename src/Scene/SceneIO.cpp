#include "SceneIO.h"
#include "../Core/Logger.h"
#include "../ModelImport/ModelImporter.h"
#include "BehaviorRegistry.h"
#include "ObjectFactory.h"
#include "SceneManager.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

json SceneIO::SerializeObject(const GameObject &obj) {
  json jObj;
  jObj["name"] = obj.name;
  jObj["position"] = {obj.position.x, obj.position.y, obj.position.z};
  jObj["rotation"] = {obj.rotation.w, obj.rotation.x, obj.rotation.y,
                      obj.rotation.z};
  jObj["scale"] = {obj.scale.x, obj.scale.y, obj.scale.z};

  jObj["useGravity"] = obj.useGravity;
  jObj["isStatic"] = obj.isStatic;
  jObj["mass"] = obj.mass;
  jObj["friction"] = obj.friction;
  jObj["restitution"] = obj.restitution;
  jObj["enableCollision"] = obj.enableCollision;
  jObj["centerOfMassOffset"] = {obj.centerOfMassOffset.x,
                                obj.centerOfMassOffset.y,
                                obj.centerOfMassOffset.z};
  jObj["angularVelocity"] = {obj.angularVelocity.x, obj.angularVelocity.y,
                             obj.angularVelocity.z};
  jObj["shape"] = static_cast<int>(obj.shape);
  jObj["collisionRadius"] = obj.collisionRadius;
  jObj["isTrigger"] = obj.isTrigger;
  jObj["scripts"] = obj.scriptNames;
  jObj["material"] = obj.material.Serialize();
  jObj["modelPath"] = obj.modelPath;
  jObj["meshIndex"] = obj.meshIndex;
  jObj["meshType"] = static_cast<int>(obj.meshType);
  jObj["parentIndex"] = obj.parentIndex;
  jObj["isFolded"] = obj.isFolded;
  jObj["isActive"] = obj.isActive;

  jObj["acousticMaterial"] = {
      {"hardness", obj.acousticMaterial.hardness},
      {"absorption", obj.acousticMaterial.absorption},
      {"isAcousticObstacle", obj.acousticMaterial.isAcousticObstacle}};

  jObj["hasAudio"] = obj.hasAudio;
  jObj["audio"] = {{"filePath", obj.audio.filePath},
                   {"volume", obj.audio.volume},
                   {"pitch", obj.audio.pitch},
                   {"looping", obj.audio.looping},
                   {"minDistance", obj.audio.minDistance},
                   {"maxDistance", obj.audio.maxDistance},
                   {"type", static_cast<int>(obj.audio.type)},
                   {"playOnAwake", obj.audio.playOnAwake},
                   {"enableDoppler", obj.audio.enableDoppler},
                   {"dopplerFactor", obj.audio.dopplerFactor},
                   {"enableReverb", obj.audio.enableReverb},
                   {"enableOcclusion", obj.audio.enableOcclusion}};

  jObj["hasCamera"] = obj.hasCamera;
  jObj["camera"] = {{"enabled", obj.camera.enabled},
                    {"fov", obj.camera.fov},
                    {"nearPlane", obj.camera.nearPlane},
                    {"farPlane", obj.camera.farPlane},
                    {"resolutionX", obj.camera.resolutionX},
                    {"resolutionY", obj.camera.resolutionY}};

  jObj["hasScreen"] = obj.hasScreen;
  jObj["screen"] = {{"enabled", obj.screen.enabled},
                    {"type", static_cast<int>(obj.screen.type)},
                    {"filePath", obj.screen.filePath},
                    {"targetCameraIndex", obj.screen.targetCameraIndex},
                    {"isVideoPlaying", obj.screen.isVideoPlaying},
                    {"brightness", obj.screen.brightness},
                    {"videoLoop", obj.screen.videoLoop},
                    {"videoPaused", obj.screen.videoPaused},
                    {"videoPlaybackSpeed", obj.screen.videoPlaybackSpeed},
                    {"videoVolume", obj.screen.videoVolume},
                    {"videoKeepAspect", obj.screen.videoKeepAspect}};

  jObj["hasWater"] = obj.hasWater;
  jObj["water"] = {{"waveSpeed", obj.water.waveSpeed},
                   {"waveStrength", obj.water.waveStrength},
                   {"shininess", obj.water.shininess},
                   {"waterColor",
                    {obj.water.waterColor.r, obj.water.waterColor.g,
                     obj.water.waterColor.b}},
                   {"waveSystem", obj.water.waveSystem},
                   {"tiling", obj.water.tiling},
                   {"gridResolution", obj.water.gridResolution},
                   {"surfaceHeight", obj.water.surfaceHeight},
                   {"depth", obj.water.depth},
                   {"liquidDensity", obj.water.liquidDensity}};

  jObj["hasSDF"] = obj.hasSDF;
  jObj["sdf"] = {{"resolution", obj.sdf.resolution}};

  jObj["is2DSprite"] = obj.is2DSprite;
  jObj["sprite"] = {{"faceCamera", obj.sprite.faceCamera},
                    {"targetCameraIndex", obj.sprite.targetCameraIndex}};

  return jObj;
}

void SceneIO::DeserializeObject(const json &jObj, GameObject &obj) {
  if (jObj.contains("shape")) {
    obj.shape = static_cast<ColliderShape>(jObj["shape"].get<int>());
  }

  if (jObj.contains("position")) {
    auto pos = jObj["position"];
    obj.position = glm::vec3(pos[0], pos[1], pos[2]);
  }

  if (jObj.contains("rotation")) {
    auto rot = jObj["rotation"];
    obj.rotation = glm::quat(rot[0], rot[1], rot[2], rot[3]);
  }

  if (jObj.contains("scale")) {
    auto sc = jObj["scale"];
    obj.scale = glm::vec3(sc[0], sc[1], sc[2]);
  }

  if (jObj.contains("useGravity"))
    obj.useGravity = jObj["useGravity"];
  if (jObj.contains("isStatic"))
    obj.isStatic = jObj["isStatic"];
  if (jObj.contains("mass"))
    obj.mass = jObj["mass"];
  if (jObj.contains("friction"))
    obj.friction = jObj["friction"];
  if (jObj.contains("restitution"))
    obj.restitution = jObj["restitution"];
  if (jObj.contains("enableCollision"))
    obj.enableCollision = jObj["enableCollision"];
  if (jObj.contains("centerOfMassOffset")) {
    auto com = jObj["centerOfMassOffset"];
    obj.centerOfMassOffset = glm::vec3(com[0], com[1], com[2]);
  }

  if (jObj.contains("angularVelocity")) {
    auto av = jObj["angularVelocity"];
    obj.angularVelocity = glm::vec3(av[0], av[1], av[2]);
  }
  if (jObj.contains("collisionRadius")) {
    obj.collisionRadius = jObj["collisionRadius"];
  }
  if (jObj.contains("isTrigger")) {
    obj.isTrigger = jObj["isTrigger"];
  }

  if (jObj.contains("scripts")) {
    for (const auto &sName : jObj["scripts"]) {
      auto behavior = BehaviorRegistry::Create(sName);
      if (behavior) {
        behavior->gameObject = nullptr;
        obj.behaviors.push_back(std::move(behavior));
        obj.scriptNames.push_back(sName);
      }
    }
  }

  if (jObj.contains("material")) {
    obj.material.Deserialize(jObj["material"]);
  }

  if (jObj.contains("parentIndex"))
    obj.parentIndex = jObj["parentIndex"];
  if (jObj.contains("isFolded"))
    obj.isFolded = jObj["isFolded"];
  if (jObj.contains("isActive"))
    obj.isActive = jObj["isActive"];

  if (jObj.contains("acousticMaterial")) {
    auto &am = jObj["acousticMaterial"];
    if (am.contains("hardness"))
      obj.acousticMaterial.hardness = am["hardness"];
    if (am.contains("absorption"))
      obj.acousticMaterial.absorption = am["absorption"];
    if (am.contains("isAcousticObstacle"))
      obj.acousticMaterial.isAcousticObstacle = am["isAcousticObstacle"];
  }

  if (jObj.contains("hasAudio"))
    obj.hasAudio = jObj["hasAudio"];
  if (jObj.contains("audio")) {
    auto &a = jObj["audio"];
    if (a.contains("filePath"))
      obj.audio.filePath = a["filePath"];
    if (a.contains("volume"))
      obj.audio.volume = a["volume"];
    if (a.contains("pitch"))
      obj.audio.pitch = a["pitch"];
    if (a.contains("looping"))
      obj.audio.looping = a["looping"];
    if (a.contains("minDistance"))
      obj.audio.minDistance = a["minDistance"];
    if (a.contains("maxDistance"))
      obj.audio.maxDistance = a["maxDistance"];
    if (a.contains("type"))
      obj.audio.type = static_cast<AudioType>(a["type"].get<int>());
    if (a.contains("playOnAwake"))
      obj.audio.playOnAwake = a["playOnAwake"];

    if (a.contains("enableDoppler"))
      obj.audio.enableDoppler = a["enableDoppler"];
    if (a.contains("dopplerFactor"))
      obj.audio.dopplerFactor = a["dopplerFactor"];
    if (a.contains("enableReverb"))
      obj.audio.enableReverb = a["enableReverb"];
    if (a.contains("enableOcclusion"))
      obj.audio.enableOcclusion = a["enableOcclusion"];
  }

  if (jObj.contains("hasCamera"))
    obj.hasCamera = jObj["hasCamera"];
  if (jObj.contains("camera")) {
    auto &c = jObj["camera"];
    if (c.contains("enabled"))
      obj.camera.enabled = c["enabled"];
    if (c.contains("fov"))
      obj.camera.fov = c["fov"];
    if (c.contains("nearPlane"))
      obj.camera.nearPlane = c["nearPlane"];
    if (c.contains("farPlane"))
      obj.camera.farPlane = c["farPlane"];
    if (c.contains("resolutionX"))
      obj.camera.resolutionX = c["resolutionX"];
    if (c.contains("resolutionY"))
      obj.camera.resolutionY = c["resolutionY"];
  }

  if (jObj.contains("hasScreen"))
    obj.hasScreen = jObj["hasScreen"];
  if (jObj.contains("screen")) {
    auto &s = jObj["screen"];
    if (s.contains("enabled"))
      obj.screen.enabled = s["enabled"];
    if (s.contains("type"))
      obj.screen.type = static_cast<ScreenType>(s["type"].get<int>());
    if (s.contains("filePath"))
      obj.screen.filePath = s["filePath"];
    if (s.contains("targetCameraIndex"))
      obj.screen.targetCameraIndex = s["targetCameraIndex"];
    if (s.contains("isVideoPlaying"))
      obj.screen.isVideoPlaying = s["isVideoPlaying"];
    if (s.contains("brightness"))
      obj.screen.brightness = s["brightness"];
    if (s.contains("videoLoop"))
      obj.screen.videoLoop = s["videoLoop"];
    if (s.contains("videoPaused"))
      obj.screen.videoPaused = s["videoPaused"];
    if (s.contains("videoPlaybackSpeed"))
      obj.screen.videoPlaybackSpeed = s["videoPlaybackSpeed"];
    if (s.contains("videoVolume"))
      obj.screen.videoVolume = s["videoVolume"];
    if (s.contains("videoKeepAspect"))
      obj.screen.videoKeepAspect = s["videoKeepAspect"];
  }

  if (jObj.contains("hasWater"))
    obj.hasWater = jObj["hasWater"];
  if (jObj.contains("water")) {
    auto &w = jObj["water"];
    if (w.contains("waveSpeed"))
      obj.water.waveSpeed = w["waveSpeed"];
    if (w.contains("waveStrength"))
      obj.water.waveStrength = w["waveStrength"];
    if (w.contains("shininess"))
      obj.water.shininess = w["shininess"];
    if (w.contains("waterColor")) {
      auto col = w["waterColor"];
      obj.water.waterColor = glm::vec3(col[0], col[1], col[2]);
    }
    if (w.contains("waveSystem"))
      obj.water.waveSystem = w["waveSystem"];
    if (w.contains("tiling"))
      obj.water.tiling = w["tiling"];
    if (w.contains("gridResolution"))
      obj.water.gridResolution = w["gridResolution"];
    if (w.contains("surfaceHeight"))
      obj.water.surfaceHeight = w["surfaceHeight"];
    if (w.contains("depth"))
      obj.water.depth = w["depth"];
    if (w.contains("liquidDensity"))
      obj.water.liquidDensity = w["liquidDensity"];

    if (obj.hasWater) {
      obj.mesh = ObjectFactory::createWaterGrid(obj.water.gridResolution);
    }
  }

  if (jObj.contains("hasSDF"))
    obj.hasSDF = jObj["hasSDF"];
  if (jObj.contains("sdf")) {
    auto &s = jObj["sdf"];
    if (s.contains("resolution"))
      obj.sdf.resolution = s["resolution"];
  }

  if (jObj.contains("is2DSprite"))
    obj.is2DSprite = jObj["is2DSprite"];
  if (jObj.contains("sprite")) {
    auto &s = jObj["sprite"];
    if (s.contains("faceCamera"))
      obj.sprite.faceCamera = s["faceCamera"];
    if (s.contains("targetCameraIndex"))
      obj.sprite.targetCameraIndex = s["targetCameraIndex"];
  }
}

void SceneIO::SavePrefab(const GameObject &obj, const std::string &path) {
  json data = SerializeObject(obj);
  std::ofstream file(path);
  if (file.is_open()) {
    file << data.dump(4);
    Logger::AddLog("Prefab saved to %s", path.c_str());
  }
}

GameObject SceneIO::LoadPrefab(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open())
    return GameObject(Mesh({}, {}, {}));

  json data = json::parse(file);

  MeshType loadedType = MeshType::None;
  if (data.contains("meshType"))
    loadedType = static_cast<MeshType>(data["meshType"].get<int>());

  std::string modelPath = "";
  if (data.contains("modelPath"))
    modelPath = data["modelPath"].get<std::string>();
  int meshIndex = -1;
  if (data.contains("meshIndex"))
    meshIndex = data["meshIndex"].get<int>();

  GameObject *objPtr = nullptr;
  if (loadedType == MeshType::Model && !modelPath.empty() && meshIndex != -1) {
    auto result = ModelImporter::Import(modelPath);
    if (result.success && meshIndex < (int)result.meshes.size()) {
      auto &meshData = result.meshes[meshIndex];
      Mesh mesh(meshData.vertices, meshData.indices, meshData.textures);
      objPtr = new GameObject(std::move(mesh), data["name"]);
      objPtr->modelPath = modelPath;
      objPtr->meshIndex = meshIndex;
      objPtr->meshType = MeshType::Model;

    } else {
      if (!result.success) {
        Logger::AddLog("[SceneIO] Prefab model import failed: %s",
                       result.error.c_str());
      } else {
        Logger::AddLog(
            "[SceneIO] Prefab mesh index %d out of bounds (count: %d)",
            meshIndex, (int)result.meshes.size());
      }
    }
  }

  if (!objPtr) {
    if (loadedType == MeshType::Cube)
      objPtr = new GameObject(ObjectFactory::createCube(), data["name"]);
    else if (loadedType == MeshType::Sphere)
      objPtr =
          new GameObject(ObjectFactory::createSphere(30, 30), data["name"]);
    else if (loadedType == MeshType::Plane)
      objPtr = new GameObject(ObjectFactory::createPlane(), data["name"]);
    else if (loadedType == MeshType::Camera)
      objPtr = new GameObject(ObjectFactory::createCameraMesh(), data["name"]);
    else
      objPtr = new GameObject(Mesh({}, {}, {}), data["name"]);
    objPtr->meshType = loadedType;
  }

  GameObject obj = std::move(*objPtr);
  delete objPtr;
  DeserializeObject(data, obj);
  return obj;
}

void Scene::Save(const std::string &path, bool silent) {
  m_Filepath = path;
  json data;

  data["physics"]["GlobalGravityEnabled"] = PhysicsEngine::GlobalGravityEnabled;
  data["physics"]["GlobalPhysicsEnabled"] = PhysicsEngine::GlobalPhysicsEnabled;
  data["physics"]["ImpulseEnabled"] = PhysicsEngine::ImpulseEnabled;
  data["physics"]["GlobalCOMEnabled"] = PhysicsEngine::GlobalCOMEnabled;
  data["physics"]["SubSteps"] = PhysicsEngine::SubSteps;
  data["physics"]["LinearDamping"] = PhysicsEngine::LinearDamping;
  data["physics"]["AngularDamping"] = PhysicsEngine::AngularDamping;
  data["physics"]["GlobalAirResistance"] = PhysicsEngine::GlobalAirResistance;
  data["physics"]["Gravity"] = {PhysicsEngine::Gravity.x,
                                PhysicsEngine::Gravity.y,
                                PhysicsEngine::Gravity.z};
  data["physics"]["GlobalAcceleration"] = {PhysicsEngine::GlobalAcceleration.x,
                                           PhysicsEngine::GlobalAcceleration.y,
                                           PhysicsEngine::GlobalAcceleration.z};

  if (auto cam = SceneManager::Get().GetMainCamera()) {
    data["main_camera"]["parentIndex"] = cam->parentIndex;
  }
  data["main_camera"]["game_camera_index"] = m_GameCameraIndex;

  for (const auto &obj : m_Objects) {
    data["objects"].push_back(SceneIO::SerializeObject(obj));
  }

  for (const auto &pl : m_PointLights) {
    json jPl;
    jPl["position"] = {pl.position.x, pl.position.y, pl.position.z};
    jPl["color"] = {pl.color.r, pl.color.g, pl.color.b, pl.color.a};
    jPl["intensity"] = pl.intensity;
    jPl["enabled"] = pl.enabled;
    jPl["castShadows"] = pl.castShadows;
    data["point_lights"].push_back(jPl);
  }

  json jFlags = json::object();
  for (auto const &[name, fd] : m_Flags) {
    jFlags[name] = {{"pos", {fd.position.x, fd.position.y, fd.position.z}},
                    {"yaw", fd.yaw},
                    {"pitch", fd.pitch}};
  }
  data["flags"] = jFlags;

  std::ofstream file(path);
  if (file.is_open()) {
    file << data.dump(4);
    if (!silent)
      Logger::AddLog("Scene saved to %s", path.c_str());
  }
}

void Scene::Load(const std::string &path) {
  m_Filepath = path;
  std::ifstream file(path);
  if (!file.is_open())
    return;

  try {
    json data = json::parse(file);
    Clear();

    if (data.contains("physics")) {
      auto &phys = data["physics"];
      if (phys.contains("GlobalGravityEnabled"))
        PhysicsEngine::GlobalGravityEnabled = phys["GlobalGravityEnabled"];
      if (phys.contains("GlobalPhysicsEnabled"))
        PhysicsEngine::GlobalPhysicsEnabled = phys["GlobalPhysicsEnabled"];
      if (phys.contains("ImpulseEnabled"))
        PhysicsEngine::ImpulseEnabled = phys["ImpulseEnabled"];
      if (phys.contains("GlobalCOMEnabled"))
        PhysicsEngine::GlobalCOMEnabled = phys["GlobalCOMEnabled"];
      if (phys.contains("SubSteps"))
        PhysicsEngine::SubSteps = phys["SubSteps"];
      if (phys.contains("LinearDamping"))
        PhysicsEngine::LinearDamping = phys["LinearDamping"];
      if (phys.contains("AngularDamping"))
        PhysicsEngine::AngularDamping = phys["AngularDamping"];
      if (phys.contains("GlobalAirResistance"))
        PhysicsEngine::GlobalAirResistance = phys["GlobalAirResistance"];

      if (phys.contains("Gravity")) {
        auto g = phys["Gravity"];
        PhysicsEngine::Gravity = glm::vec3(g[0], g[1], g[2]);
      }
      if (phys.contains("GlobalAcceleration")) {
        auto ga = phys["GlobalAcceleration"];
        PhysicsEngine::GlobalAcceleration = glm::vec3(ga[0], ga[1], ga[2]);
      }
    }

    if (data.contains("main_camera")) {
      if (data["main_camera"].contains("game_camera_index")) {
        m_GameCameraIndex = data["main_camera"]["game_camera_index"];
      }
      if (auto cam = SceneManager::Get().GetMainCamera()) {
        if (data["main_camera"].contains("parentIndex")) {
          cam->parentIndex = data["main_camera"]["parentIndex"];
        }
      }
    }

    std::string sceneDir = std::filesystem::path(path).parent_path().string();
    std::string derivedRoot =
        std::filesystem::path(sceneDir).parent_path().string();
    std::string projectRoot =
        m_ProjectRoot.empty() ? derivedRoot : m_ProjectRoot;

    if (data.contains("objects")) {
      for (const auto &jObj : data["objects"]) {
        MeshType loadedType = MeshType::None;
        if (jObj.contains("meshType"))
          loadedType = static_cast<MeshType>(jObj["meshType"].get<int>());

        std::string modelPath = "";
        if (jObj.contains("modelPath"))
          modelPath = jObj["modelPath"].get<std::string>();
        int meshIndex = -1;
        if (jObj.contains("meshIndex"))
          meshIndex = jObj["meshIndex"].get<int>();

        GameObject *objPtr = nullptr;
        if (loadedType == MeshType::Model && !modelPath.empty() &&
            meshIndex != -1) {

          std::string resolvedPath = modelPath;
          if (!std::filesystem::path(modelPath).is_absolute() &&
              !std::filesystem::exists(modelPath)) {

            std::string fromProject = projectRoot + "/" + modelPath;
            if (std::filesystem::exists(fromProject)) {
              resolvedPath = fromProject;
            } else if (projectRoot != derivedRoot) {

              std::string fromDerived = derivedRoot + "/" + modelPath;
              if (std::filesystem::exists(fromDerived)) {
                resolvedPath = fromDerived;
              }
            }

            if (resolvedPath == modelPath) {
              std::string fromScene = sceneDir + "/" + modelPath;
              if (std::filesystem::exists(fromScene)) {
                resolvedPath = fromScene;
              }
            }
          }

          auto result = ModelImporter::Import(resolvedPath);
          if (result.success && meshIndex < (int)result.meshes.size()) {
            auto &meshData = result.meshes[meshIndex];
            Mesh mesh(meshData.vertices, meshData.indices, meshData.textures);
            objPtr = new GameObject(std::move(mesh), jObj["name"]);
            objPtr->modelPath = modelPath;
            objPtr->meshIndex = meshIndex;
            objPtr->meshType = MeshType::Model;
            objPtr->meshType = MeshType::Model;

            if (!jObj.contains("material")) {
              objPtr->material.albedo = meshData.albedo;
              objPtr->material.metallic = meshData.metallic;
              objPtr->material.roughness = meshData.roughness;
              objPtr->material.useTexture = !meshData.textures.empty();
            }
          } else {
            if (!result.success) {
              Logger::AddLog("[ERROR] Failed to import model %s: %s",
                             resolvedPath.c_str(), result.error.c_str());
            } else {
              Logger::AddLog("[ERROR] Mesh index %d out of bounds for model %s "
                             "(count: %d)",
                             meshIndex, resolvedPath.c_str(),
                             (int)result.meshes.size());
            }
          }
        }

        if (!objPtr) {
          if (loadedType == MeshType::Cube)
            objPtr = new GameObject(ObjectFactory::createCube(), jObj["name"]);
          else if (loadedType == MeshType::Sphere)
            objPtr = new GameObject(ObjectFactory::createSphere(30, 30),
                                    jObj["name"]);
          else if (loadedType == MeshType::Plane) {
            if (jObj.contains("hasWater") && jObj["hasWater"].get<bool>()) {
              int res = jObj.contains("water") &&
                                jObj["water"].contains("gridResolution")
                            ? jObj["water"]["gridResolution"].get<int>()
                            : 400;
              objPtr = new GameObject(ObjectFactory::createWaterGrid(res),
                                      jObj["name"]);
            } else {
              objPtr =
                  new GameObject(ObjectFactory::createPlane(), jObj["name"]);
            }
          } else if (loadedType == MeshType::Camera)
            objPtr =
                new GameObject(ObjectFactory::createCameraMesh(), jObj["name"]);
          else
            objPtr = new GameObject(Mesh({}, {}, {}), jObj["name"]);
          objPtr->meshType = loadedType;
        }

        GameObject &obj = *objPtr;
        SceneIO::DeserializeObject(jObj, obj);
        AddObject(std::move(obj));
        delete objPtr;
      }
    }

    if (data.contains("point_lights")) {
      for (const auto &jPl : data["point_lights"]) {
        PointLight *pl = CreatePointLight();
        auto pos = jPl["position"];
        pl->position = glm::vec3(pos[0], pos[1], pos[2]);
        auto col = jPl["color"];
        pl->color = glm::vec4(col[0], col[1], col[2], col[3]);
        pl->intensity = jPl["intensity"];
        pl->enabled = jPl["enabled"];
        if (jPl.contains("castShadows"))
          pl->castShadows = jPl["castShadows"].get<bool>();
      }
    }

    if (data.contains("flags")) {
      auto jFlags = data["flags"];
      for (auto it = jFlags.begin(); it != jFlags.end(); ++it) {
        auto &v = it.value();
        glm::vec3 pos(0.0f);
        float yaw = -90.0f, pitch = 0.0f;

        if (v.contains("pos")) {
          auto p = v["pos"];
          pos = glm::vec3(p[0], p[1], p[2]);
          if (v.contains("yaw"))
            yaw = v["yaw"].get<float>();
          if (v.contains("pitch"))
            pitch = v["pitch"].get<float>();
        } else {

          pos = glm::vec3(v[0], v[1], v[2]);
        }
        AddFlag(it.key(), pos, yaw, pitch);
      }
    }

    Logger::AddLog("Scene loaded from %s", path.c_str());
  } catch (const std::exception &e) {
    Logger::AddLog("[ERROR] Scene load failed: %s", e.what());
  }
}
