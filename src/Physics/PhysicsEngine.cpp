#include "PhysicsEngine.h"
#include "../Core/ThreadManager.h"
#include "../Scene/Scene.h"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


static float blinnWyvillHelper(float x) {
    float x2 = x * x;
    if (x2 >= 1.0f) return 0.0f;
    return (1.0f - x2) * (1.0f - x2) * (1.0f - x2);
}

static glm::vec3 gerstnerWaveHelper(glm::vec2 pos, glm::vec2 direction, float amplitude, float wavelength, float speed, float time) {
    float k = 2.0f * 3.14159f / wavelength;
    float c = speed;
    glm::vec2 d = glm::normalize(direction);
    float f = k * (glm::dot(d, pos) - c * time);
    float steepness = 0.6f;
    
    return glm::vec3(
        d.x * amplitude * cos(f) * steepness,
        amplitude * sin(f),
        d.y * amplitude * cos(f) * steepness
    );
}

static float getSampledWaveHeight(glm::vec2 pos, float time, float waveSpeed, float waveStrength, int waveSystem, float tiling) {
    glm::vec2 tiledPos = pos * tiling;
    float t = time * waveSpeed;

    if (waveSystem == 0) {
        
        glm::vec2 w1 = glm::vec2(tiledPos.x + tiledPos.y * 2.5f, tiledPos.y - tiledPos.x * 1.8f) * 0.6f;
        glm::vec2 w2 = glm::vec2(tiledPos.x * 1.3f - tiledPos.y * 3.2f, tiledPos.y * 1.5f + tiledPos.x * 2.8f) * 0.4f;
        glm::vec2 w3 = glm::vec2(tiledPos.x * 2.2f + tiledPos.y * 1.7f, tiledPos.y * 1.9f - tiledPos.x * 2.3f) * 0.5f;
        glm::vec2 w4 = glm::vec2(tiledPos.x * 3.5f - tiledPos.y * 0.9f, tiledPos.y * 2.7f + tiledPos.x * 1.2f) * 0.7f;
        glm::vec2 w5 = glm::vec2(tiledPos.x * 4.1f + tiledPos.y * 0.5f, tiledPos.y * 3.3f - tiledPos.x * 1.6f) * 0.8f;
        
        auto mod2 = [](float x, float y) { return x - y * floor(x / y); };
        float t1 = mod2(w1.x + t * 0.8f, 2.0f) - 1.0f;
        float t2 = mod2(w2.y - t * 1.2f, 2.0f) - 1.0f;
        float t3 = mod2(w3.x + t * 0.6f, 2.0f) - 1.0f;
        float t4 = mod2(w4.y - t * 1.5f, 2.0f) - 1.0f;
        float t5 = mod2(w5.x + t * 0.9f, 2.0f) - 1.0f;
        
        float h = (blinnWyvillHelper(t1) * 0.22f + blinnWyvillHelper(t2) * 0.16f + 
                   blinnWyvillHelper(t3) * 0.11f + blinnWyvillHelper(t4) * 0.07f + 
                   blinnWyvillHelper(t5) * 0.05f);
        return h * waveStrength;
    } else {
        
        glm::vec3 wave(0.0f);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(1.0f, 0.3f), 0.15f, 8.0f, 2.0f, t);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(-0.7f, 0.9f), 0.12f, 6.5f, 1.8f, t);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(0.5f, -1.0f), 0.08f, 4.0f, 2.5f, t);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(-0.9f, -0.4f), 0.06f, 3.5f, 2.2f, t);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(0.8f, 0.6f), 0.04f, 2.0f, 3.0f, t);
        wave += gerstnerWaveHelper(tiledPos, glm::vec2(-0.3f, -0.8f), 0.03f, 1.5f, 3.5f, t);
        return wave.y * waveStrength;
    }
}


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

void PhysicsEngine::Update(float deltaTime, float time, std::vector<GameObject>& objects) {
    if (deltaTime <= 0.0f || !GlobalPhysicsEnabled) return;

    float subDeltaTime = deltaTime / (float)SubSteps;

    for (int step = 0; step < SubSteps; ++step) {
        
        struct WaterVolume { 
            AABB box; 
            float surfaceY; 
            float bottomY; 
            float density; 
            float waveSpeed;
            float waveStrength;
            int waveSystem;
            float tiling;
        };
        std::vector<WaterVolume> waterVolumes;
        for (const auto& wObj : objects) {
            if (wObj.hasWater && wObj.isActive) {
                AABB worldAABB = GetTransformedAABB(wObj.collider, wObj.position, wObj.rotation, wObj.scale);
                float sY = wObj.position.y + wObj.water.surfaceHeight;
                waterVolumes.push_back({
                    worldAABB, sY, sY - wObj.water.depth, wObj.water.liquidDensity,
                    wObj.water.waveSpeed, wObj.water.waveStrength, 
                    wObj.water.waveSystem, wObj.water.tiling
                });
            }
        }
        
        auto integrateFunc = [&](int i) {
            auto& obj = objects[i];
            if (!obj.isActive || obj.isStatic) return;

            
            for (const auto& water : waterVolumes) {
                const AABB& waterBox = water.box;
                
                AABB objAABB = GetTransformedAABB(obj.collider, obj.position, obj.rotation, obj.scale);
                
                
                if (objAABB.max.x > waterBox.min.x && objAABB.min.x < waterBox.max.x &&
                    objAABB.max.z > waterBox.min.z && objAABB.min.z < waterBox.max.z) 
                {
                    
                    float waveH = getSampledWaveHeight(glm::vec2(obj.position.x, obj.position.z), 
                                                     time, water.waveSpeed, water.waveStrength, 
                                                     water.waveSystem, water.tiling);
                    
                    float dynamicSurfaceY = water.surfaceY + waveH;
                    float waterBottomY = water.bottomY;

                    if (objAABB.min.y < dynamicSurfaceY && objAABB.max.y > waterBottomY) {
                        float objHeight = objAABB.max.y - objAABB.min.y;
                        if (objHeight < 0.0001f) objHeight = 0.0001f;
                        
                        float effectiveSurfaceY = std::min(dynamicSurfaceY, objAABB.max.y);
                        float effectiveBottomY = std::max(waterBottomY, objAABB.min.y);
                        float submergedHeight = std::max(0.0f, effectiveSurfaceY - effectiveBottomY);
                        float submergedRatio = std::min(1.0f, submergedHeight / objHeight);
                        
                        float objVolume = 1.0f;
                        if (obj.shape == ColliderShape::Box) {
                            objVolume = (obj.collider.max.x - obj.collider.min.x) * obj.scale.x *
                                        (obj.collider.max.y - obj.collider.min.y) * obj.scale.y *
                                        (obj.collider.max.z - obj.collider.min.z) * obj.scale.z;
                        } else if (obj.shape == ColliderShape::Sphere) {
                            float r = obj.collisionRadius * obj.scale.x;
                            objVolume = (4.0f / 3.0f) * glm::pi<float>() * r * r * r;
                        }
                        
                        if (objVolume > 0.0f && obj.mass > 0.0f) {
                            float buoyantForce = water.density * (objVolume * submergedRatio) * std::abs(Gravity.y);
                            
                            
                            obj.velocity.y += (buoyantForce / obj.mass) * subDeltaTime;
                            
                            
                            float dragCoeff = 0.8f; 
                            obj.velocity *= std::pow(dragCoeff, subDeltaTime * 60.0f);
                            obj.angularVelocity *= std::pow(dragCoeff, subDeltaTime * 60.0f);
                        }
                    }
                }
            }
            
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
        };

        if (ThreadManager::IsEnabled()) {
            ThreadManager::ParallelFor(0, (int)objects.size(), integrateFunc);
        } else {
            for (int i = 0; i < (int)objects.size(); ++i) integrateFunc(i);
        }

        
        for (size_t i = 0; i < objects.size(); ++i) {
            auto& objA = objects[i];
            if (!objA.isActive || !objA.enableCollision) continue;

            OBB obbA = GetGameObjectOBB(objA);

            for (size_t j = i + 1; j < objects.size(); ++j) {
                auto& objB = objects[j];
                if (!objB.isActive || !objB.enableCollision) continue;
                
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
                    if (objA.isTrigger || objB.isTrigger || objA.hasWater || objB.hasWater) continue;
                    
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

                            
                            for (auto& behavior : objA.behaviors) {
                                if (behavior && behavior->enabled) behavior->OnCollisionEnter(&objB);
                            }
                            for (auto& behavior : objB.behaviors) {
                                if (behavior && behavior->enabled) behavior->OnCollisionEnter(&objA);
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

bool PhysicsEngine::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, std::vector<GameObject>& objects, RaycastHit& outHit) {
    float closestDist = maxDistance;
    bool hitFound = false;
    glm::vec3 norm(0, 0, 0);

    for (auto& obj : objects) {
        if (!obj.isActive || !obj.enableCollision) continue;

        AABB worldAABB = GetTransformedAABB(obj.collider, obj.position, obj.rotation, obj.scale);
        
        float tmin = (worldAABB.min.x - origin.x) / direction.x;
        float tmax = (worldAABB.max.x - origin.x) / direction.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (worldAABB.min.y - origin.y) / direction.y;
        float tymax = (worldAABB.max.y - origin.y) / direction.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) continue;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (worldAABB.min.z - origin.z) / direction.z;
        float tzmax = (worldAABB.max.z - origin.z) / direction.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) continue;
        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        if (tmin > 0.0f && tmin < closestDist) {
            closestDist = tmin;
            outHit.object = &obj;
            outHit.point = origin + direction * tmin;
            outHit.distance = tmin;
            
            
            glm::vec3 center = (worldAABB.max + worldAABB.min) * 0.5f;
            glm::vec3 rel = outHit.point - center;
            glm::vec3 extents = (worldAABB.max - worldAABB.min) * 0.5f;
            glm::vec3 absRel = glm::abs(rel / extents);
            if (absRel.x > absRel.y && absRel.x > absRel.z) outHit.normal = glm::vec3(rel.x > 0 ? 1 : -1, 0, 0);
            else if (absRel.y > absRel.z) outHit.normal = glm::vec3(0, rel.y > 0 ? 1 : -1, 0);
            else outHit.normal = glm::vec3(0, 0, rel.z > 0 ? 1 : -1);

            hitFound = true;
        }
    }

    return hitFound;
}
