#pragma once
#include <glm/glm.hpp>
#include <vector>

class Scene;

class StreamingManager {
public:
  static void Update(const glm::vec3 &cameraPos, Scene &scene);
  static void SetStreamingRadius(float radius) { s_StreamingRadius = radius; }
  static float GetStreamingRadius() { return s_StreamingRadius; }

  static bool s_EnableStreaming;

private:
  static float s_StreamingRadius;
};
