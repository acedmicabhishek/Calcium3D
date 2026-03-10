#define MINIAUDIO_IMPLEMENTATION
#include "AudioEngine.h"
#include "../../include/miniaudio/miniaudio.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

ma_engine *AudioEngine::s_Engine = nullptr;
bool AudioEngine::s_Initialized = false;
glm::vec3 AudioEngine::s_PrevListenerPos = glm::vec3(0.0f);
int AudioEngine::s_MaxRealVoices = 32;
bool AudioEngine::s_CullWhenInaudible = true;

static constexpr float SPEED_OF_SOUND = 343.0f;

void AudioEngine::Init() {
  if (s_Initialized)
    return;

  s_Engine = new ma_engine();
  ma_result result = ma_engine_init(NULL, s_Engine);
  if (result != MA_SUCCESS) {
    Logger::AddLog("[AudioEngine] Failed to initialize audio engine.");
    delete s_Engine;
    s_Engine = nullptr;
    return;
  }

  s_Initialized = true;
  Logger::AddLog("[AudioEngine] Initialized with physics-based audio.");
}

void AudioEngine::Shutdown() {
  if (!s_Initialized)
    return;

  ma_engine_uninit(s_Engine);
  delete s_Engine;
  s_Engine = nullptr;
  s_Initialized = false;
}

bool AudioEngine::RayIntersectsAABB(const glm::vec3 &rayOrigin,
                                    const glm::vec3 &rayDir,
                                    const glm::vec3 &boxMin,
                                    const glm::vec3 &boxMax, float &tHit) {
  float tmin = -1e30f, tmax = 1e30f;

  for (int i = 0; i < 3; i++) {
    if (std::abs(rayDir[i]) < 1e-8f) {
      if (rayOrigin[i] < boxMin[i] || rayOrigin[i] > boxMax[i])
        return false;
    } else {
      float invD = 1.0f / rayDir[i];
      float t1 = (boxMin[i] - rayOrigin[i]) * invD;
      float t2 = (boxMax[i] - rayOrigin[i]) * invD;
      if (t1 > t2)
        std::swap(t1, t2);
      tmin = std::max(tmin, t1);
      tmax = std::min(tmax, t2);
      if (tmin > tmax)
        return false;
    }
  }

  if (tmax < 0.0f)
    return false;

  tHit = (tmin >= 0.0f) ? tmin : tmax;
  return true;
}

float AudioEngine::ComputeOcclusion(Scene *scene, const glm::vec3 &sourcePos,
                                    const glm::vec3 &listenerPos,
                                    int sourceIndex) {
  if (glm::length(listenerPos - sourcePos) < 0.001f)
    return 0.0f;

  glm::vec3 forward = glm::normalize(sourcePos - listenerPos);

  glm::vec3 up =
      std::abs(forward.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
  glm::vec3 right = glm::normalize(glm::cross(forward, up));
  up = glm::normalize(glm::cross(right, forward));

  float listenerRadius = 0.5f;

  glm::vec3 targetPoints[5] = {listenerPos, listenerPos + up * listenerRadius,
                               listenerPos - up * listenerRadius,
                               listenerPos + right * listenerRadius,
                               listenerPos - right * listenerRadius};

  float totalOcclusion = 0.0f;
  auto &objects = scene->GetObjects();

  for (int r = 0; r < 5; r++) {
    glm::vec3 dir = targetPoints[r] - sourcePos;
    float totalDist = glm::length(dir);
    dir /= totalDist;

    float rayOcclusion = 0.0f;

    for (int i = 0; i < (int)objects.size(); i++) {
      if (i == sourceIndex)
        continue;
      if (!objects[i].isActive)
        continue;
      if (!objects[i].acousticMaterial.isAcousticObstacle)
        continue;
      if (objects[i].meshType == MeshType::None)
        continue;

      glm::mat4 globalT = scene->GetGlobalTransform(i);
      glm::vec3 globalPos = glm::vec3(globalT[3]);
      glm::vec3 globalScale(glm::length(glm::vec3(globalT[0])),
                            glm::length(glm::vec3(globalT[1])),
                            glm::length(glm::vec3(globalT[2])));
      glm::mat3 rotMat = glm::mat3(globalT);
      rotMat[0] /= globalScale.x;
      rotMat[1] /= globalScale.y;
      rotMat[2] /= globalScale.z;

      AABB worldAABB = PhysicsEngine::GetTransformedAABB(
          objects[i].collider, globalPos, glm::quat_cast(rotMat), globalScale);

      float tHit;
      if (RayIntersectsAABB(sourcePos, dir, worldAABB.min, worldAABB.max,
                            tHit)) {
        if (tHit < totalDist) {
          float blockAmount = objects[i].acousticMaterial.hardness * 0.4f +
                              objects[i].acousticMaterial.absorption * 1.5f;
          rayOcclusion += 0.2f + blockAmount;
        }
      }
    }
    totalOcclusion += std::min(rayOcclusion, 1.0f);
  }

  return std::min(totalOcclusion / 5.0f, 1.0f);
}

float AudioEngine::ComputeReverb(Scene *scene, const glm::vec3 &sourcePos,
                                 float &outEchoDelay, float &outEchoDecay) {

  static const glm::vec3 rayDirs[] = {{1, 0, 0},
                                      {-1, 0, 0},
                                      {0, 1, 0},
                                      {0, -1, 0},
                                      {0, 0, 1},
                                      {0, 0, -1},
                                      {0.577f, 0.577f, 0.577f},
                                      {-0.577f, 0.577f, 0.577f},
                                      {0.577f, -0.577f, 0.577f},
                                      {-0.577f, -0.577f, 0.577f},
                                      {0.577f, 0.577f, -0.577f},
                                      {-0.577f, 0.577f, -0.577f},
                                      {0.577f, -0.577f, -0.577f},
                                      {-0.577f, -0.577f, -0.577f}};
  static const int numRays = 14;
  static const float maxRayDist = 50.0f;

  auto &objects = scene->GetObjects();

  float totalDistSum = 0.0f;
  float totalHardness = 0.0f;
  int hitCount = 0;

  for (int r = 0; r < numRays; r++) {
    float closestHit = maxRayDist;
    float closestHardness = 0.0f;

    for (int i = 0; i < (int)objects.size(); i++) {
      if (!objects[i].isActive)
        continue;
      if (objects[i].meshType == MeshType::None)
        continue;
      if (!objects[i].acousticMaterial.isAcousticObstacle)
        continue;

      AABB worldAABB = PhysicsEngine::GetTransformedAABB(
          objects[i].collider, objects[i].position, objects[i].rotation,
          objects[i].scale);

      float tHit;
      if (RayIntersectsAABB(sourcePos, rayDirs[r], worldAABB.min, worldAABB.max,
                            tHit)) {
        if (tHit < closestHit && tHit > 0.01f) {
          closestHit = tHit;
          closestHardness = objects[i].acousticMaterial.hardness;
        }
      }
    }

    if (closestHit < maxRayDist) {
      totalDistSum += closestHit;
      totalHardness += closestHardness;
      hitCount++;
    }
  }

  if (hitCount == 0) {
    outEchoDelay = 0.0f;
    outEchoDecay = 0.0f;
    return 0.0f;
  }

  float avgDist = totalDistSum / (float)hitCount;
  float avgHardness = totalHardness / (float)hitCount;
  float hitRatio = (float)hitCount / (float)numRays;

  float roomFactor = 1.0f - std::min(avgDist / maxRayDist, 1.0f);
  float reverbMix = roomFactor * hitRatio * avgHardness;
  reverbMix = std::clamp(reverbMix, 0.0f, 0.8f);

  outEchoDelay = (avgDist * 2.0f) / SPEED_OF_SOUND;
  outEchoDelay = std::clamp(outEchoDelay, 0.01f, 0.5f);

  outEchoDecay = avgHardness * hitRatio * 0.7f;
  outEchoDecay = std::clamp(outEchoDecay, 0.0f, 0.6f);

  return reverbMix;
}

float AudioEngine::ComputeDopplerPitch(const glm::vec3 &sourcePos,
                                       const glm::vec3 &prevSourcePos,
                                       const glm::vec3 &listenerPos,
                                       const glm::vec3 &prevListenerPos,
                                       float deltaTime, float dopplerFactor) {
  if (deltaTime <= 0.0001f)
    return 1.0f;

  glm::vec3 sourceVel = (sourcePos - prevSourcePos) / deltaTime;

  glm::vec3 listenerVel = (listenerPos - prevListenerPos) / deltaTime;

  glm::vec3 toListener = listenerPos - sourcePos;
  float dist = glm::length(toListener);
  if (dist < 0.001f)
    return 1.0f;
  toListener /= dist;

  float vSource = glm::dot(sourceVel, toListener);
  float vListener = glm::dot(listenerVel, toListener);

  float numerator = SPEED_OF_SOUND - vListener * dopplerFactor;
  float denominator = SPEED_OF_SOUND - vSource * dopplerFactor;

  denominator = std::max(denominator, SPEED_OF_SOUND * 0.1f);
  numerator = std::max(numerator, SPEED_OF_SOUND * 0.1f);

  float dopplerPitch = numerator / denominator;
  return std::clamp(dopplerPitch, 0.5f, 2.0f);
}

void AudioEngine::Update(Scene *scene, const glm::vec3 &listenerPos,
                         const glm::vec3 &listenerDir,
                         const glm::vec3 &listenerUp, float deltaTime) {
  if (!s_Initialized || !scene)
    return;

  ma_engine_listener_set_position(s_Engine, 0, listenerPos.x, listenerPos.y,
                                  listenerPos.z);
  ma_engine_listener_set_direction(s_Engine, 0, listenerDir.x, listenerDir.y,
                                   listenerDir.z);
  ma_engine_listener_set_world_up(s_Engine, 0, listenerUp.x, listenerUp.y,
                                  listenerUp.z);

  auto &objects = scene->GetObjects();

  
  struct AudioPriority {
    int index;
    float distance;
    float priority; 
  };
  std::vector<AudioPriority> activeAudio;

  for (int idx = 0; idx < (int)objects.size(); idx++) {
    auto &obj = objects[idx];

    if (!obj.hasAudio || obj.audio.filePath.empty()) {
      if (obj.audio.pSource)
        StopObjectAudio(obj);
      continue;
    }

    glm::mat4 audioT = scene->GetGlobalTransform(idx);
    glm::vec3 worldPos = glm::vec3(audioT[3]);
    float dist = glm::distance(worldPos, listenerPos);

    
    if (s_CullWhenInaudible && obj.audio.type == AudioType::Directional &&
        dist > obj.audio.maxDistance) {
      if (obj.audio.pSource)
        StopObjectAudio(obj);
      obj.prevPosition = worldPos;
      continue;
    }

    if (!obj.audio.playing && !obj.audio.pSource) {
      obj.prevPosition = worldPos;
      continue;
    }

    
    float priority = obj.audio.volume / std::max(dist, 0.1f);
    if (obj.audio.type == AudioType::Ambience)
      priority += 100.0f; 
    activeAudio.push_back({idx, dist, priority});
  }

  
  std::sort(activeAudio.begin(), activeAudio.end(),
            [](const AudioPriority &a, const AudioPriority &b) {
              return a.priority > b.priority;
            });

  for (int i = 0; i < (int)activeAudio.size(); i++) {
    auto &obj = objects[activeAudio[i].index];
    int idx = activeAudio[i].index;
    glm::mat4 audioT = scene->GetGlobalTransform(idx);
    glm::vec3 worldPos = glm::vec3(audioT[3]);

    
    if (i >= s_MaxRealVoices) {
      if (obj.audio.pSource) {
        StopObjectAudio(obj);
        obj.audio.playing = true; 
      }
      obj.prevPosition = worldPos;
      continue;
    }

    
    if (obj.audio.playing && !obj.audio.pSource) {
      PlayObjectAudio(obj);
    } else if (!obj.audio.playing && obj.audio.pSource) {
      StopObjectAudio(obj);
    }

    
    if (obj.audio.pSource) {
      ma_sound *sound = (ma_sound *)obj.audio.pSource;

      if (obj.audio.type == AudioType::Directional) {
        ma_sound_set_position(sound, worldPos.x, worldPos.y, worldPos.z);
        ma_sound_set_min_distance(sound, obj.audio.minDistance);
        ma_sound_set_max_distance(sound, obj.audio.maxDistance);
      }

      float dopplerPitch = 1.0f;
      if (obj.audio.enableDoppler && obj.audio.type == AudioType::Directional) {
        dopplerPitch = ComputeDopplerPitch(worldPos, obj.prevPosition,
                                           listenerPos, s_PrevListenerPos,
                                           deltaTime, obj.audio.dopplerFactor);
      }

      float occlusionVolMult = 1.0f;
      float occlusionPitchMult = 1.0f;
      if (obj.audio.enableOcclusion &&
          obj.audio.type == AudioType::Directional) {
        obj.audio.occlusionFactor =
            ComputeOcclusion(scene, worldPos, listenerPos, idx);
        occlusionVolMult = std::max(0.0f, 1.0f - obj.audio.occlusionFactor);
        occlusionPitchMult =
            std::max(0.5f, 1.0f - (obj.audio.occlusionFactor * 0.5f));
      }

      if (obj.audio.enableReverb && obj.audio.type == AudioType::Directional) {
        float echoDelay, echoDecay;
        obj.audio.reverbMix =
            ComputeReverb(scene, worldPos, echoDelay, echoDecay);
        obj.audio.echoDelay = echoDelay;
        obj.audio.echoDecay = echoDecay;
        float reverbBoost = 1.0f + obj.audio.reverbMix * 0.3f;
        occlusionVolMult *= reverbBoost;
      }

      float finalVolume = obj.audio.volume * occlusionVolMult;
      float finalPitch = obj.audio.pitch * dopplerPitch * occlusionPitchMult;

      ma_sound_set_volume(sound, std::clamp(finalVolume, 0.0f, 2.0f));
      ma_sound_set_pitch(sound, std::clamp(finalPitch, 0.1f, 4.0f));
      ma_sound_set_looping(sound, obj.audio.looping);
    }

    obj.prevPosition = worldPos;
  }

  s_PrevListenerPos = listenerPos;
}

void AudioEngine::PlayObjectAudio(GameObject &obj) {
  if (!s_Initialized)
    return;
  if (obj.audio.filePath.empty())
    return;

  if (!std::filesystem::exists(obj.audio.filePath)) {
    return;
  }

  if (obj.audio.pSource) {
    StopObjectAudio(obj);
  }

  ma_sound *sound = new ma_sound();

  ma_result result = ma_sound_init_from_file(
      s_Engine, obj.audio.filePath.c_str(), 0, NULL, NULL, sound);
  if (result != MA_SUCCESS) {
    Logger::AddLog("[AudioEngine] Failed to load sound: %s",
                   obj.audio.filePath.c_str());
    delete sound;
    return;
  }

  if (obj.audio.type == AudioType::Ambience) {
    ma_sound_set_spatialization_enabled(sound, MA_FALSE);
  } else {
    ma_sound_set_spatialization_enabled(sound, MA_TRUE);

    ma_sound_set_position(sound, obj.position.x, obj.position.y,
                          obj.position.z);
    ma_sound_set_min_distance(sound, obj.audio.minDistance);
    ma_sound_set_max_distance(sound, obj.audio.maxDistance);

    ma_sound_set_doppler_factor(sound, obj.audio.dopplerFactor);
  }

  ma_sound_set_volume(sound, obj.audio.volume);
  ma_sound_set_pitch(sound, obj.audio.pitch);
  ma_sound_set_looping(sound, obj.audio.looping);

  ma_sound_start(sound);
  obj.audio.pSource = sound;
  obj.audio.playing = true;
  obj.prevPosition = obj.position;
}

void AudioEngine::StopObjectAudio(GameObject &obj) {
  if (obj.audio.pSource) {
    ma_sound *sound = (ma_sound *)obj.audio.pSource;
    ma_sound_stop(sound);
    ma_sound_uninit(sound);
    delete sound;
    obj.audio.pSource = nullptr;
  }
  obj.audio.playing = false;
}
