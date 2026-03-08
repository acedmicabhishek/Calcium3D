#include "Frustum.h"

Frustum Frustum::CreateFrustumFromCamera(const glm::mat4 &viewProj) {
  Frustum frustum;

  frustum.leftFace.normal.x = viewProj[0][3] + viewProj[0][0];
  frustum.leftFace.normal.y = viewProj[1][3] + viewProj[1][0];
  frustum.leftFace.normal.z = viewProj[2][3] + viewProj[2][0];
  frustum.leftFace.distance = viewProj[3][3] + viewProj[3][0];

  frustum.rightFace.normal.x = viewProj[0][3] - viewProj[0][0];
  frustum.rightFace.normal.y = viewProj[1][3] - viewProj[1][0];
  frustum.rightFace.normal.z = viewProj[2][3] - viewProj[2][0];
  frustum.rightFace.distance = viewProj[3][3] - viewProj[3][0];

  frustum.bottomFace.normal.x = viewProj[0][3] + viewProj[0][1];
  frustum.bottomFace.normal.y = viewProj[1][3] + viewProj[1][1];
  frustum.bottomFace.normal.z = viewProj[2][3] + viewProj[2][1];
  frustum.bottomFace.distance = viewProj[3][3] + viewProj[3][1];

  frustum.topFace.normal.x = viewProj[0][3] - viewProj[0][1];
  frustum.topFace.normal.y = viewProj[1][3] - viewProj[1][1];
  frustum.topFace.normal.z = viewProj[2][3] - viewProj[2][1];
  frustum.topFace.distance = viewProj[3][3] - viewProj[3][1];

  frustum.nearFace.normal.x = viewProj[0][3] + viewProj[0][2];
  frustum.nearFace.normal.y = viewProj[1][3] + viewProj[1][2];
  frustum.nearFace.normal.z = viewProj[2][3] + viewProj[2][2];
  frustum.nearFace.distance = viewProj[3][3] + viewProj[3][2];

  frustum.farFace.normal.x = viewProj[0][3] - viewProj[0][2];
  frustum.farFace.normal.y = viewProj[1][3] - viewProj[1][2];
  frustum.farFace.normal.z = viewProj[2][3] - viewProj[2][2];
  frustum.farFace.distance = viewProj[3][3] - viewProj[3][2];

  auto normalizePlane = [](Plane &p) {
    float mag = glm::length(p.normal);
    p.normal /= mag;
    p.distance /= mag;
  };

  normalizePlane(frustum.leftFace);
  normalizePlane(frustum.rightFace);
  normalizePlane(frustum.bottomFace);
  normalizePlane(frustum.topFace);
  normalizePlane(frustum.nearFace);
  normalizePlane(frustum.farFace);

  return frustum;
}

bool Frustum::IsOnFrustum(const glm::vec3 &min, const glm::vec3 &max) const {
  auto checkPlane = [&](const Plane &plane) {
    glm::vec3 vmin, vmax;

    if (plane.normal.x > 0) {
      vmin.x = min.x;
      vmax.x = max.x;
    } else {
      vmin.x = max.x;
      vmax.x = min.x;
    }

    if (plane.normal.y > 0) {
      vmin.y = min.y;
      vmax.y = max.y;
    } else {
      vmin.y = max.y;
      vmax.y = min.y;
    }

    if (plane.normal.z > 0) {
      vmin.z = min.z;
      vmax.z = max.z;
    } else {
      vmin.z = max.z;
      vmax.z = min.z;
    }

    if (glm::dot(plane.normal, vmax) + plane.distance < 0) {
      return false;
    }
    return true;
  };

  return checkPlane(leftFace) && checkPlane(rightFace) &&
         checkPlane(bottomFace) && checkPlane(topFace) &&
         checkPlane(nearFace) && checkPlane(farFace);
}
