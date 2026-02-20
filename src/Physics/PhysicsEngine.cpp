#include "PhysicsEngine.h"
#include "../Scene/Scene.h"
#include <algorithm>

bool PhysicsEngine::GlobalGravityEnabled = true;
glm::vec3 PhysicsEngine::Gravity = glm::vec3(0.0f, -9.81f, 0.0f);
glm::vec3 PhysicsEngine::GlobalAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
bool PhysicsEngine::GlobalPhysicsEnabled = true;
bool PhysicsEngine::ImpulseEnabled = true;
int PhysicsEngine::SubSteps = 1;
float PhysicsEngine::LinearDamping = 0.95f; 
float PhysicsEngine::AngularDamping = 0.90f;
float PhysicsEngine::GlobalAirResistance = 0.1f;
bool PhysicsEngine::GlobalCOMEnabled = true;

bool PhysicsEngine::CheckCollision(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

AABB PhysicsEngine::GetTransformedAABB(const AABB& localAABB, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
    
    glm::vec3 corners[8] = {
        localAABB.min,
        {localAABB.min.x, localAABB.min.y, localAABB.max.z},
        {localAABB.min.x, localAABB.max.y, localAABB.min.z},
        {localAABB.min.x, localAABB.max.y, localAABB.max.z},
        {localAABB.max.x, localAABB.min.y, localAABB.min.z},
        {localAABB.max.x, localAABB.min.y, localAABB.max.z},
        {localAABB.max.x, localAABB.max.y, localAABB.min.z},
        localAABB.max
    };

    glm::vec3 worldMin(1e10f), worldMax(-1e10f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * 
                      glm::mat4_cast(rotation) * 
                      glm::scale(glm::mat4(1.0f), scale);

    
    for (int i = 0; i < 8; ++i) {
        glm::vec4 worldCorner = model * glm::vec4(corners[i], 1.0f);
        worldMin = glm::min(worldMin, glm::vec3(worldCorner));
        worldMax = glm::max(worldMax, glm::vec3(worldCorner));
    }

    return AABB(worldMin, worldMax);
}


struct OBB {
    glm::vec3 center;
    glm::vec3 axes[3];
    glm::vec3 halfExtents;
};

OBB GetGameObjectOBB(const GameObject& obj) {
    OBB obb;
    glm::vec3 localCenter = (obj.collider.min + obj.collider.max) * 0.5f;
    glm::mat4 rot = glm::mat4_cast(obj.rotation);
    obb.center = obj.position + glm::vec3(rot * glm::vec4(localCenter, 1.0f));
    obb.axes[0] = glm::normalize(glm::vec3(rot[0]));
    obb.axes[1] = glm::normalize(glm::vec3(rot[1]));
    obb.axes[2] = glm::normalize(glm::vec3(rot[2]));
    obb.halfExtents = (obj.collider.max - obj.collider.min) * 0.5f * obj.scale;
    return obb;
}

bool TestOBBOBB(const OBB& a, const OBB& b, glm::vec3& outNormal, float& outPenetration) {
    float minOverlap = 1e10f;
    glm::vec3 smallestAxis(0.0f);

    glm::vec3 axesToTest[15];
    for (int i = 0; i < 3; ++i) axesToTest[i] = a.axes[i];
    for (int i = 0; i < 3; ++i) axesToTest[i + 3] = b.axes[i];
    
    int axisCount = 6;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 crossAxis = glm::cross(a.axes[i], b.axes[j]);
            if (glm::length(crossAxis) > 0.01f) { 
                axesToTest[axisCount++] = glm::normalize(crossAxis);
            }
        }
    }

    glm::vec3 L = b.center - a.center;

    for (int i = 0; i < axisCount; i++) {
        glm::vec3 axis = axesToTest[i];
        if (glm::length(axis) < 0.001f) continue;

        float ra = a.halfExtents.x * glm::abs(glm::dot(a.axes[0], axis)) +
                   a.halfExtents.y * glm::abs(glm::dot(a.axes[1], axis)) +
                   a.halfExtents.z * glm::abs(glm::dot(a.axes[2], axis));
        float rb = b.halfExtents.x * glm::abs(glm::dot(b.axes[0], axis)) +
                   b.halfExtents.y * glm::abs(glm::dot(b.axes[1], axis)) +
                   b.halfExtents.z * glm::abs(glm::dot(b.axes[2], axis));

        float overlap = ra + rb - glm::abs(glm::dot(L, axis));
        if (overlap < 0) return false;

        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    outPenetration = minOverlap;
    
    outNormal = (glm::dot(L, smallestAxis) > 0) ? -smallestAxis : smallestAxis;
    return true;
}

bool TestSphereSphere(const glm::vec3& centerA, float radiusA, const glm::vec3& centerB, float radiusB, glm::vec3& outNormal, float& outPenetration) {
    glm::vec3 distance = centerA - centerB;
    float distSq = glm::dot(distance, distance);
    float radiusSum = radiusA + radiusB;
    
    if (distSq >= radiusSum * radiusSum) return false;
    
    float dist = std::sqrt(distSq);
    if (dist == 0.0f) {
        outNormal = glm::vec3(0, 1, 0); 
        outPenetration = radiusSum;
    } else {
        outNormal = distance / dist; 
        outPenetration = radiusSum - dist;
    }
    return true;
}

bool TestOBBSphere(const OBB& obb, const glm::vec3& sphereCenter, float sphereRadius, glm::vec3& outNormal, float& outPenetration, bool obbIsA) {
    
    glm::vec3 d = sphereCenter - obb.center;
    glm::vec3 closestPoint = obb.center;
    
    for (int i = 0; i < 3; i++) {
        float dist = glm::dot(d, obb.axes[i]);
        if (dist > obb.halfExtents[i]) dist = obb.halfExtents[i];
        if (dist < -obb.halfExtents[i]) dist = -obb.halfExtents[i];
        closestPoint += dist * obb.axes[i];
    }
    
    glm::vec3 v = sphereCenter - closestPoint;
    float distSq = glm::dot(v, v);
    
    if (distSq > sphereRadius * sphereRadius) return false;
    
    float dist = std::sqrt(distSq);
    
    
    
    if (dist == 0.0f) {
        
        float minPenetration = 1e10f;
        glm::vec3 bestAxis(0.0f);
        
        for (int i = 0; i < 3; i++) {
            float projection = glm::dot(d, obb.axes[i]);
            float pen = obb.halfExtents[i] - std::abs(projection);
            if (pen < minPenetration) {
                minPenetration = pen;
                bestAxis = obb.axes[i] * (projection > 0 ? 1.0f : -1.0f);
            }
        }
        
        outNormal = bestAxis;
        outPenetration = sphereRadius + minPenetration;
    } else {
        outNormal = v / dist;
        outPenetration = sphereRadius - dist;
    }
    
    
    
    
    
    
    if (obbIsA) {
        outNormal = -outNormal;
    }
    
    return true;
}


glm::mat3 GetBoxInertiaTensor(const glm::vec3& halfExtents, float mass) {
    float x2 = (halfExtents.x * 2.0f) * (halfExtents.x * 2.0f);
    float y2 = (halfExtents.y * 2.0f) * (halfExtents.y * 2.0f);
    float z2 = (halfExtents.z * 2.0f) * (halfExtents.z * 2.0f);
    float f = mass / 12.0f;
    return glm::mat3(
        f * (y2 + z2), 0, 0,
        0, f * (x2 + z2), 0,
        0, 0, f * (x2 + y2)
    );
}

void PhysicsEngine::Update(float deltaTime, std::vector<GameObject>& objects) {
    if (deltaTime <= 0.0f || !GlobalPhysicsEnabled) return;

    float subDeltaTime = deltaTime / (float)SubSteps;

    for (int step = 0; step < SubSteps; ++step) {
        
        for (auto& obj : objects) {
            if (obj.isStatic) continue;

            
            if (GlobalGravityEnabled && obj.useGravity) {
                obj.velocity += Gravity * subDeltaTime;
            }
            obj.velocity += GlobalAcceleration * subDeltaTime;

            
            obj.velocity += obj.acceleration * subDeltaTime;
            obj.velocity *= std::pow(LinearDamping, subDeltaTime * 60.0f); 

            
            if (GlobalAirResistance > 0.0f && glm::length(obj.velocity) > 0.001f) {
                float speed = glm::length(obj.velocity);
                float deceleration = GlobalAirResistance * subDeltaTime;
                obj.velocity = glm::normalize(obj.velocity) * std::max(0.0f, speed - deceleration);
            }

            
            if (glm::length(obj.torque) > 0.001f) {
                OBB obb = GetGameObjectOBB(obj);
                glm::mat3 rot = glm::mat4_cast(obj.rotation);
                glm::mat3 I_local = GetBoxInertiaTensor(obb.halfExtents, obj.mass);
                glm::mat3 invI = rot * glm::inverse(I_local) * glm::transpose(rot);
                obj.angularVelocity += (invI * obj.torque) * subDeltaTime;
            }
            obj.angularVelocity *= std::pow(AngularDamping, subDeltaTime * 60.0f);

            
            if (GlobalAirResistance > 0.0f && glm::length(obj.angularVelocity) > 0.001f) {
                float rotSpeed = glm::length(obj.angularVelocity);
                float rotDecel = GlobalAirResistance * subDeltaTime;
                obj.angularVelocity = glm::normalize(obj.angularVelocity) * std::max(0.0f, rotSpeed - rotDecel);
            }

            
            obj.position += obj.velocity * subDeltaTime;

            if (glm::length(obj.angularVelocity) > 0.0001f) {
                float angle = glm::length(obj.angularVelocity) * subDeltaTime;
                glm::vec3 axis = glm::normalize(obj.angularVelocity);
                glm::quat deltaRot = glm::angleAxis(angle, axis);
                
                if (GlobalCOMEnabled && glm::length(obj.centerOfMassOffset) > 0.001f) {
                    glm::vec3 worldCOM = obj.position + obj.rotation * obj.centerOfMassOffset;
                    obj.rotation = glm::normalize(deltaRot * obj.rotation);
                    glm::vec3 newWorldCOM = obj.position + obj.rotation * obj.centerOfMassOffset;
                    obj.position += (worldCOM - newWorldCOM);
                } else {
                    obj.rotation = glm::normalize(deltaRot * obj.rotation);
                }
            }
        }

        
        for (size_t i = 0; i < objects.size(); ++i) {
            auto& objA = objects[i];
            if (!objA.enableCollision) continue;

            OBB obbA = GetGameObjectOBB(objA);

            for (size_t j = i + 1; j < objects.size(); ++j) {
                auto& objB = objects[j];
                if (!objB.enableCollision) continue;
                
                OBB obbB = GetGameObjectOBB(objB);

                glm::vec3 normal;
                float penetration;
                bool collided = false;

                if (objA.shape == ColliderShape::Box && objB.shape == ColliderShape::Box) {
                    collided = TestOBBOBB(obbA, obbB, normal, penetration);
                } else if (objA.shape == ColliderShape::Sphere && objB.shape == ColliderShape::Sphere) {
                    float rA = objA.collisionRadius * objA.scale.x;
                    float rB = objB.collisionRadius * objB.scale.x;
                    collided = TestSphereSphere(objA.position, rA, objB.position, rB, normal, penetration);
                } else if (objA.shape == ColliderShape::Box && objB.shape == ColliderShape::Sphere) {
                    float rB = objB.collisionRadius * objB.scale.x;
                    collided = TestOBBSphere(obbA, objB.position, rB, normal, penetration, true);
                } else if (objA.shape == ColliderShape::Sphere && objB.shape == ColliderShape::Box) {
                    float rA = objA.collisionRadius * objA.scale.x;
                    collided = TestOBBSphere(obbB, objA.position, rA, normal, penetration, false);
                }

                if (collided) {
                    float invMassA = objA.isStatic ? 0.0f : (1.0f / objA.mass);
                    float invMassB = objB.isStatic ? 0.0f : (1.0f / objB.mass);

                    if (invMassA + invMassB == 0.0f) continue; 

                    
                    const float slop = 0.02f; 
                    const float percent = 0.2f; 
                    glm::vec3 correction = (std::max(penetration - slop, 0.0f) / (invMassA + invMassB)) * percent * normal;
                    
                    if (!objA.isStatic) objA.position += correction * invMassA;
                    if (!objB.isStatic) objB.position -= correction * invMassB;

                    if (ImpulseEnabled) {
                        glm::mat3 invIA(0.0f);
                        if (!objA.isStatic) {
                            glm::mat3 rotA = glm::mat4_cast(objA.rotation);
                            glm::mat3 I_localA;
                            if (objA.shape == ColliderShape::Sphere) {
                                float r = objA.collisionRadius * objA.scale.x;
                                float iStr = (2.0f / 5.0f) * objA.mass * r * r;
                                I_localA = glm::mat3(iStr, 0, 0, 0, iStr, 0, 0, 0, iStr);
                            } else {
                                I_localA = GetBoxInertiaTensor(obbA.halfExtents, objA.mass);
                            }
                            invIA = rotA * glm::inverse(I_localA) * glm::transpose(rotA);
                        }

                        glm::mat3 invIB(0.0f);
                        if (!objB.isStatic) {
                            glm::mat3 rotB = glm::mat4_cast(objB.rotation);
                            glm::mat3 I_localB;
                            if (objB.shape == ColliderShape::Sphere) {
                                float r = objB.collisionRadius * objB.scale.x;
                                float iStr = (2.0f / 5.0f) * objB.mass * r * r;
                                I_localB = glm::mat3(iStr, 0, 0, 0, iStr, 0, 0, 0, iStr);
                            } else {
                                I_localB = GetBoxInertiaTensor(obbB.halfExtents, objB.mass);
                            }
                            invIB = rotB * glm::inverse(I_localB) * glm::transpose(rotB);
                        }

                        
                        glm::vec3 worldCOM_A = objA.position;
                        if (GlobalCOMEnabled) worldCOM_A += objA.rotation * objA.centerOfMassOffset;
                        glm::vec3 worldCOM_B = objB.position;
                        if (GlobalCOMEnabled && !objB.isStatic) worldCOM_B += objB.rotation * objB.centerOfMassOffset;
                        
                        glm::vec3 contactPoint;
                        if (objA.shape == ColliderShape::Sphere && objB.shape == ColliderShape::Sphere) {
                            contactPoint = objB.position + normal * (objB.collisionRadius * objB.scale.x); 
                        } else if (objA.shape == ColliderShape::Sphere && objB.shape == ColliderShape::Box) {
                            contactPoint = objA.position - normal * (objA.collisionRadius * objA.scale.x); 
                        } else if (objA.shape == ColliderShape::Box && objB.shape == ColliderShape::Sphere) {
                            contactPoint = objB.position + normal * (objB.collisionRadius * objB.scale.x);
                        } else {
                            
                            float maxDotA = 0.0f;
                            for (int k = 0; k < 3; ++k) maxDotA = std::max(maxDotA, std::abs(glm::dot(obbA.axes[k], normal)));
                            
                            float maxDotB = 0.0f;
                            for (int k = 0; k < 3; ++k) maxDotB = std::max(maxDotB, std::abs(glm::dot(obbB.axes[k], normal)));
                            
                            if (maxDotA >= maxDotB) {
                                contactPoint = obbB.center;
                                for (int k = 0; k < 3; k++) {
                                    float dotP = glm::dot(obbB.axes[k], normal);
                                    float sign = (dotP > 0.0f) ? 1.0f : -1.0f;
                                    float mask = (std::abs(dotP) > 0.05f) ? 1.0f : 0.0f; 
                                    contactPoint += obbB.axes[k] * obbB.halfExtents[k] * sign * mask;
                                }
                            } else {
                                contactPoint = obbA.center;
                                for (int k = 0; k < 3; k++) {
                                    float dotP = glm::dot(obbA.axes[k], -normal);
                                    float sign = (dotP > 0.0f) ? 1.0f : -1.0f;
                                    float mask = (std::abs(dotP) > 0.05f) ? 1.0f : 0.0f; 
                                    contactPoint += obbA.axes[k] * obbA.halfExtents[k] * sign * mask;
                                }
                            }
                        }

                        glm::vec3 rA = contactPoint - worldCOM_A;
                        glm::vec3 rB = contactPoint - worldCOM_B;

                        glm::vec3 vA = (objA.isStatic ? glm::vec3(0.0f) : objA.velocity) + glm::cross(objA.angularVelocity, rA);
                        glm::vec3 vB = (objB.isStatic ? glm::vec3(0.0f) : objB.velocity) + glm::cross(objB.angularVelocity, rB);
                        glm::vec3 relVel = vA - vB;
                        float velAlongNormal = glm::dot(relVel, normal);

                        if (velAlongNormal < 0.001f) { 
                            glm::vec3 crossAN = glm::cross(rA, normal);
                            glm::vec3 crossBN = glm::cross(rB, normal);
                            float angularTermA = glm::dot(invIA * crossAN, crossAN);
                            float angularTermB = objB.isStatic ? 0.0f : glm::dot(invIB * crossBN, crossBN);

                            
                            float e = std::min(objA.restitution, objB.restitution);
                            if (std::abs(velAlongNormal) < 0.2f) e = 0.0f; 

                            float j_impulse = -(1.0f + e) * velAlongNormal;
                            j_impulse /= (invMassA + invMassB + angularTermA + angularTermB);

                            glm::vec3 impulse = j_impulse * normal;
                            if (!objA.isStatic) {
                                objA.velocity += impulse * invMassA;
                                objA.angularVelocity += invIA * glm::cross(rA, impulse);
                            }
                            if (!objB.isStatic) {
                                objB.velocity -= impulse * invMassB;
                                objB.angularVelocity -= invIB * glm::cross(rB, impulse);
                            }

                            
                            glm::vec3 tangent = relVel - (glm::dot(relVel, normal) * normal);
                            if (glm::length(tangent) > 0.001f) {
                                tangent = glm::normalize(tangent);
                                glm::vec3 crossAT = glm::cross(rA, tangent);
                                glm::vec3 crossBT = glm::cross(rB, tangent);
                                float angularTermAT = glm::dot(invIA * crossAT, crossAT);
                                float angularTermBT = objB.isStatic ? 0.0f : glm::dot(invIB * crossBT, crossBT);

                                float frictionCoeff = std::sqrt(objA.friction * objB.friction);
                                
                                
                                float jt = -glm::dot(relVel, tangent);
                                jt /= (invMassA + invMassB); 

                                
                                float normalForceImpulse = 0.0f;
                                if (GlobalGravityEnabled) {
                                    if (!objA.isStatic && objA.useGravity) {
                                        normalForceImpulse += glm::abs(glm::dot(Gravity, normal)) * objA.mass * subDeltaTime;
                                    }
                                    if (!objB.isStatic && objB.useGravity) {
                                        normalForceImpulse += glm::abs(glm::dot(Gravity, normal)) * objB.mass * subDeltaTime;
                                    }
                                }

                                float frictionLimit = (std::max(0.0f, j_impulse) + normalForceImpulse) * frictionCoeff;
                                jt = std::max(-frictionLimit, std::min(jt, frictionLimit));

                                glm::vec3 frictionImpulse = jt * tangent;
                                if (!objA.isStatic) {
                                    objA.velocity += frictionImpulse * invMassA;
                                    
                                    objA.angularVelocity += invIA * glm::cross(rA, frictionImpulse) * 0.1f;
                                }
                                if (!objB.isStatic) {
                                    objB.velocity -= frictionImpulse * invMassB;
                                    objB.angularVelocity -= invIB * glm::cross(rB, frictionImpulse) * 0.1f;
                                }
                            }

                        }
                    }
                    obbA = GetGameObjectOBB(objA);
                }
            }
        }

        
        for (auto& obj : objects) {
            if (obj.isStatic) continue;

            
            const float MAX_VEL = 100.0f;
            const float MAX_ANG = 50.0f;
            if (glm::length(obj.velocity) > MAX_VEL) obj.velocity = glm::normalize(obj.velocity) * MAX_VEL;
            if (glm::length(obj.angularVelocity) > MAX_ANG) obj.angularVelocity = glm::normalize(obj.angularVelocity) * MAX_ANG;

            
            if (glm::length(obj.velocity) < 0.02f && glm::length(obj.angularVelocity) < 0.02f) {
                obj.velocity = glm::vec3(0.0f);
                obj.angularVelocity = glm::vec3(0.0f);
            }
        }
    }

    
    for (auto& obj : objects) {
        obj.acceleration = glm::vec3(0.0f);
        obj.torque = glm::vec3(0.0f);
    }
}
