# Calcium3D Physics Engine Implementation Guide

This document details the architecture, mathematics, and implementation of the custom rigid-body physics engine built for Calcium3D.

## 1. Core Architecture & Integration

The physics engine is driven by the `PhysicsEngine::Update()` loop, which processes all `GameObject` entities in the scene. 

### Sub-Stepping
To ensure stability during high-speed collisions and complex stacking, the engine divides the frame delta time into multiple `SubSteps` (default is 8). All integration, collision detection, and resolution occur within these micro-frames, drastically reducing tunneling (objects passing through each other) and improving the accuracy of the impulse solver.

### Euler Integration
The engine uses Semi-Implicit Euler integration to update positions and rotations:
1. **Linear:** `velocity += (acceleration + gravity) * dt`, then `position += velocity * dt`.
2. **Angular:** `angularVelocity += (InverseInertiaTensor * torque) * dt`, then rotation is updated using quaternions (`deltaRot = angleAxis(length(angularVelocity) * dt, normalize(angularVelocity))`).

All transient forces (`acceleration` and `torque`) are cleared at the end of every frame (not sub-step) to allow continuous force application.

## 2. Collision Geometry & Detection

The engine supports two primary collision shapes (`ColliderShape`), defined per `GameObject`:

### Oriented Bounding Boxes (OBB)
- **Mathematical Representation:** Defined by a center point, 3 orthogonal local axes, and half-extents (dimensions divided by 2).
- **Collision Detection:** Utilizes the **Separating Axis Theorem (SAT)**. The engine projects both OBBs onto 15 potential separating axes (3 from A, 3 from B, and 9 cross products). If any axis shows a gap, there is no collision. The axis with the minimum overlap becomes the collision normal.

### Spheres
- **Mathematical Representation:** Defined by a center point and a `collisionRadius` (scaled by the object's local scale).
- **Collision Detection:** 
  - **Sphere vs Sphere:** Simple distance check between centers squared against `(radiusA + radiusB)^2`.
  - **OBB vs Sphere:** Calculates the closest point on or inside the OBB to the sphere's center by clamping the sphere's local coordinates to the OBB's half-extents. It then checks the distance from the sphere's center to this closest point.

## 3. Collision Resolution (Impulses)

When a collision is detected, the engine applies instantaneous changes in velocity (impulses) to prevent penetration and simulate bouncing/friction.

### The Impulse Equation
The engine calculates a scalar impulse $j$ representing the magnitude of the force required to separate the objects:

$$j = \frac{-(1 + e) \cdot \vec{v}_{rel} \cdot \vec{n}}{\frac{1}{m_A} + \frac{1}{m_B} + [(\vec{I}^{-1}_A (\vec{r}_A \times \vec{n})) \times \vec{r}_A] \cdot \vec{n} + [(\vec{I}^{-1}_B (\vec{r}_B \times \vec{n})) \times \vec{r}_B] \cdot \vec{n}}$$

- $e$ is the coefficient of restitution (bounciness).
- $\vec{v}_{rel}$ is the relative velocity at the contact point.
- $\vec{n}$ is the collision normal.
- $\vec{r}$ vectors are the offsets from the Center of Mass to the contact point.
- $\vec{I}^{-1}$ is the inverse inertia tensor in world space.

### Contact Point Geometry
Calculating accurate contact points is critical for realistic rotational torque:
- **OBB vs OBB (Incident Extrema):** The engine determines which box face was hit (Reference) and which box corner hit it (Incident). It geometrically finds the exact corner of the incident box furthest along the normal. This guarantees realistic, off-center hits that cause boxes to tip over naturally, preventing them from magically balancing on their edges.
- **Sphere Collisions:** The contact point is mathematically pure, lying exactly on the surface of the hull along the collision normal.

## 4. Friction Dynamics

The engine models Coulomb friction to handle sliding objects.

### Tangent Space Impulse
After the normal separating impulse is applied, the solver calculates the remaining relative velocity perpendicular to the normal (the slippage).

### Simplified Linear Friction
Because the engine uses a single-point SAT contact approximation, applying full frictional torque to the very bottom edge of a sliding box causes artificial rolling (converting sliding energy into tumbling energy). To fix this, **the friction equation omits angular denominator terms**. Friction purely destroys linear translation sliding, and applies a heavily scaled-down (10%) torque backward, ensuring objects slide to a reliable halt without tumbling infinitely.

### Gravity-Driven Normal Force
Standard formulations base the friction limit on the collision impulse (`j_impulse * friction`). However, objects perfectly at rest generate near-zero impact impulses, causing them to slide forever on sloped surfaces without friction. 
Calcium3D explicitly tracks gravitational `Normal Force` (the component of gravity pushing the mass into the surface) and incorporates it into the friction cap, meaning resting objects experience mathematically accurate dragging friction.

## 5. Advanced Mechanics

### Global Center of Mass (COM) Offset
The engine allows shifting the physical center of mass relative to the visual mesh center. 
When calculating `rA` and `rB` for torque calculations, the contact point relative offset is measured against this shifted COM. During the integration step, rotations are pivoted tightly around the shifted COM by correcting the positional drift across the quaternion multiplication.

### Inertia Tensors
- **Boxes:** $I = \frac{1}{12} m (w^2 + d^2)$
- **Spheres:** $I = \frac{2}{5} m r^2$  (Provides mathematically perfect rolling resistance down slopes).
The local inertia tensor is transformed into world-space every sub-step using the object's rotation matrix: $I^{-1}_{world} = R \cdot I^{-1}_{local} \cdot R^T$.

### Velocity Sleep & Clamping
To prevent micro-jitter and float imprecision explosions:
- **Sleep Threshold:** If an object's linear and angular velocity falls below `0.02f` at the end of all sub-steps, it is hard-clamped to zero (put to sleep).
- **Speed Clamping:** An absolute upper limit (`MAX_VEL = 100.0f`, `MAX_ANG = 50.0f`) prevents the physics simulation from mathematically detonating if two objects get deeply intertwined.

### Air Resistance
A simple linear drag is applied per-tick independent of collisions. `velocity` is reduced by a flat deceleration scalar (`GlobalAirResistance * dt`) until it reaches zero.
