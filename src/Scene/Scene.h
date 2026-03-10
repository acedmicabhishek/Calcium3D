#ifndef SCENE_H
#define SCENE_H

#include "../Physics/PhysicsEngine.h"
#include "../Renderer/Material.h"
#include "Behavior.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include <memory>
#include <vector>

#include <any>

#ifdef None
#undef None
#endif
#ifdef Status
#undef Status
#endif

enum class ColliderShape { Box, Sphere };
enum class MeshType { None, Cube, Sphere, Plane, Model, Camera, Water };
enum class AudioType { Directional, Ambience };
enum class ScreenType { None, Image, Video, CameraFeed };

struct AcousticMaterial {
  float hardness = 0.5f;
  float absorption = 0.3f;
  bool isAcousticObstacle = true;
};

struct AudioComponent {
  std::string filePath = "";
  float volume = 1.0f;
  float pitch = 1.0f;
  bool looping = false;
  float minDistance = 1.0f;
  float maxDistance = 100.0f;
  AudioType type = AudioType::Directional;
  bool playOnAwake = true;
  bool playing = false;

  bool enableDoppler = true;
  float dopplerFactor = 1.0f;

  bool enableReverb = true;
  float reverbMix = 0.0f;
  float echoDelay = 0.0f;
  float echoDecay = 0.0f;

  bool enableOcclusion = true;
  float occlusionFactor = 0.0f;

  void *pSource = nullptr;
  void *pEchoSource = nullptr;
};

struct CameraComponent {
  bool enabled = false;
  float fov = 45.0f;
  float nearPlane = 0.1f;
  float farPlane = 1000.0f;

  unsigned int fbo = 0;
  unsigned int renderTexture = 0;
  unsigned int depthBuffer = 0;
  int resolutionX = 1024;
  int resolutionY = 1024;

  std::shared_ptr<Camera> cam = nullptr;

  bool isDebugCamera = false;
  int targetCullingCameraIndex = -1;
};

struct ScreenComponent {
  bool enabled = false;
  ScreenType type = ScreenType::None;
  std::string filePath = "";
  int targetCameraIndex = -1;
  unsigned int textureID = 0;
  bool isVideoPlaying = false;
  float videoTime = 0.0f;
  std::shared_ptr<void> videoPlayerHandle;
  float brightness = 1.0f;

  bool videoLoop = true;
  bool videoPaused = true;
  float videoPlaybackSpeed = 1.0f;
  float videoVolume = 1.0f;
  bool videoKeepAspect = true;
};

struct WaterComponent {
  float waveSpeed = 1.0f;
  float waveStrength = 0.5f;
  float shininess = 64.0f;
  glm::vec3 waterColor = glm::vec3(0.0f, 0.4f, 0.8f);
  int waveSystem = 0;
  float tiling = 1.0f;
  int gridResolution = 200;

  float surfaceHeight = 0.0f;
  float depth = 5.0f;
  float liquidDensity = 1.0f;
};

struct SDFComponent {
  bool enabled = false;
  unsigned int textureID = 0;
  glm::vec3 minP;
  glm::vec3 maxP;
  int resolution = 32;
};

struct GameObject {
  Mesh mesh;
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
  std::string name;

  ColliderShape shape = ColliderShape::Box;
  float collisionRadius = 0.5f;

  int parentIndex = -1;
  bool isFolded = false;
  bool isActive = true;

  bool useGravity = false;
  bool isStatic = false;
  float mass = 1.0f;
  float friction = 0.5f;
  float restitution = 0.5f;
  bool enableCollision = true;
  glm::vec3 centerOfMassOffset = glm::vec3(0.0f);

  glm::vec3 velocity;
  glm::vec3 acceleration;
  glm::vec3 angularVelocity;
  glm::vec3 torque;
  AABB collider;
  bool isOccluder = false;

  std::vector<std::shared_ptr<Behavior>> behaviors;
  std::vector<std::string> scriptNames;
  Material material;

  AcousticMaterial acousticMaterial;

  bool hasAudio = false;
  AudioComponent audio;

  bool hasCamera = false;
  CameraComponent camera;

  bool hasScreen = false;
  ScreenComponent screen;

  bool isTrigger = false;

  bool hasWater = false;
  WaterComponent water;

  bool hasSDF = false;
  SDFComponent sdf;

  glm::vec3 prevPosition = glm::vec3(0.0f);

  std::string modelPath = "";
  int meshIndex = -1;
  MeshType meshType = MeshType::None;
  bool isStreamedOut = false;

  void ApplyImpulse(const glm::vec3 &impulse) {
    if (!isStatic && mass > 0.0f) {
      velocity += impulse / mass;
    }
  }

  GameObject(Mesh m, const std::string &n = "Object")
      : mesh(std::move(m)), position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f),
        scale(1.0f), name(n), shape(ColliderShape::Box), collisionRadius(0.5f),
        isActive(true), useGravity(false), isStatic(false), mass(1.0f),
        friction(0.5f), restitution(0.5f), enableCollision(true),
        centerOfMassOffset(0.0f), velocity(0.0f), acceleration(0.0f),
        angularVelocity(0.0f), torque(0.0f), hasAudio(false), hasCamera(false),
        hasScreen(false), hasWater(false), hasSDF(false) {

    if (!mesh.vertices.empty()) {
      glm::vec3 minExtent = mesh.vertices[0].position;
      glm::vec3 maxExtent = mesh.vertices[0].position;
      for (const auto &v : mesh.vertices) {
        minExtent = glm::min(minExtent, v.position);
        maxExtent = glm::max(maxExtent, v.position);
      }
      collider = AABB(minExtent, maxExtent);
    }
    if (hasCamera && !camera.cam) {
      camera.cam = std::make_shared<Camera>(camera.resolutionX,
                                            camera.resolutionY, position);
    }
  }
};

class Scene {
public:
  Scene();
  ~Scene();

  void Update(float dt, float time);

  void AddObject(GameObject object);
  void RemoveObject(int index);
  void RemoveObjectTree(int index);
  void DuplicateObjectTree(int index, int newParentIndex = -1);
  void Clear();

  std::vector<GameObject> &GetObjects() { return m_Objects; }

  struct Light {
    glm::vec3 position;
    glm::vec4 color;
    float intensity;
    bool enabled;
  };
  struct PointLight {
    glm::vec3 position;
    glm::vec4 color;
    float intensity;
    bool enabled;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    bool castShadows = false;
  };

  PointLight *CreatePointLight();
  void RemovePointLight(int index);
  std::vector<PointLight> &GetPointLights() { return m_PointLights; }

  void Save(const std::string &path, bool silent = false);
  void Load(const std::string &path);

  const std::string &GetFilepath() const { return m_Filepath; }
  void SetFilepath(const std::string &path) { m_Filepath = path; }

  glm::mat4 GetGlobalTransform(int objectIndex) const;

  struct FlagData {
    glm::vec3 position = glm::vec3(0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
  };

  void AddFlag(const std::string &name, const glm::vec3 &position,
               float yaw = -90.0f, float pitch = 0.0f);
  void RemoveFlag(const std::string &name);
  FlagData GetFlag(const std::string &name) const;
  const std::map<std::string, FlagData> &GetFlags() const { return m_Flags; }

  int GetGameCameraIndex() const { return m_GameCameraIndex; }
  void SetGameCameraIndex(int index) { m_GameCameraIndex = index; }

  const std::string &GetProjectRoot() const { return m_ProjectRoot; }
  void SetProjectRoot(const std::string &root) { m_ProjectRoot = root; }

public:
  PhysicsEngine physicsEngine;

  void SetProperty(const std::string &name, const std::any &value) {
    m_Blackboard[name] = value;
  }
  std::any GetProperty(const std::string &name) const {
    auto it = m_Blackboard.find(name);
    if (it != m_Blackboard.end())
      return it->second;
    return std::any();
  }
  bool HasProperty(const std::string &name) const {
    return m_Blackboard.find(name) != m_Blackboard.end();
  }

private:
  std::string m_Filepath = "";
  std::string m_ProjectRoot = "";
  std::vector<GameObject> m_Objects;
  std::vector<PointLight> m_PointLights;
  std::map<std::string, FlagData> m_Flags;
  std::map<std::string, std::any> m_Blackboard;
  int m_GameCameraIndex = -1;
};

#endif
