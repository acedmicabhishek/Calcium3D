#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "../Scene/Scene.h"
#include <glm/glm.hpp>

struct ma_engine;
struct ma_sound;

class AudioEngine {
public:
  static void Init();
  static void Shutdown();
  static void Update(Scene *scene, const glm::vec3 &listenerPos,
                     const glm::vec3 &listenerDir, const glm::vec3 &listenerUp,
                     float deltaTime);

  static void PlayObjectAudio(GameObject &obj);
  static void StopObjectAudio(GameObject &obj);

  
  static int s_MaxRealVoices;
  static bool s_CullWhenInaudible;

private:
  static ma_engine *s_Engine;
  static bool s_Initialized;

  static glm::vec3 s_PrevListenerPos;

  static float ComputeOcclusion(Scene *scene, const glm::vec3 &sourcePos,
                                const glm::vec3 &listenerPos, int sourceIndex);
  static float ComputeReverb(Scene *scene, const glm::vec3 &sourcePos,
                             float &outEchoDelay, float &outEchoDecay);
  static float ComputeDopplerPitch(const glm::vec3 &sourcePos,
                                   const glm::vec3 &prevSourcePos,
                                   const glm::vec3 &listenerPos,
                                   const glm::vec3 &prevListenerPos,
                                   float deltaTime, float dopplerFactor);

  static bool RayIntersectsAABB(const glm::vec3 &rayOrigin,
                                const glm::vec3 &rayDir,
                                const glm::vec3 &boxMin,
                                const glm::vec3 &boxMax, float &tHit);
};

#endif
