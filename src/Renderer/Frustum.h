#pragma once
#include <glm/glm.hpp>

struct Plane {
  glm::vec3 normal = {0.f, 1.f, 0.f};
  float distance = 0.f;

  Plane() = default;
  Plane(const glm::vec3 &p1, const glm::vec3 &norm)
      : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}

  float getSignedDistanceToPoint(const glm::vec3 &point) const {
    return glm::dot(normal, point) - distance;
  }
};

struct Frustum {
  Plane topFace;
  Plane bottomFace;
  Plane rightFace;
  Plane leftFace;
  Plane farFace;
  Plane nearFace;

  static Frustum CreateFrustumFromCamera(const glm::mat4 &viewProj);
  bool IsOnFrustum(const glm::vec3 &min, const glm::vec3 &max) const;
  bool IsSphereOnFrustum(const glm::vec3 &center, float radius) const;
};
