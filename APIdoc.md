# Calcium3D Programming API Guide

## Getting Started

All custom scripts in Calcium3D inherit from the `Behavior` class. To use the API, include `C3DprogrammingApi/C3D.h`.

```cpp
#include "C3DprogrammingApi/C3D.h"

class MyScript : public Behavior {
public:
    void OnStart() override { /* Triggered when object enters scene */ }
    void OnUpdate(float dt) override { /* Triggered every frame */ }
    
    // Lifecycle Callbacks [NEW]
    void OnCollisionEnter(GameObject* other) override {
        C3D::Log::Info("Hit " + other->name);
    }
    void OnTriggerEnter(GameObject* other) override {
        // Handle trigger logic (e.g., area entry)
    }
};
```

---

## Core Systems

### Object Manipulation (`C3D::Object`)
Control the physical properties and hierarchy of GameObjects.

- `SetPosition(obj, pos)` / `GetPosition(obj)`
- `SetRotation(obj, quat)` / `GetRotation(obj)`
- `SetScale(obj, scale)` / `GetScale(obj)`
- `SetActive(obj, bool)` / `IsActive(obj)`
- `SetTag(obj, "Enemy")` / `GetTag(obj)`
- `Destroy(index)` / `Duplicate(index)`

### Physics (`C3D::Physics`)
Apply forces and perform raycasts.

- `AddForce(obj, force)`
- `AddImpulse(obj, impulse)`
- `SetVelocity(obj, velocity)`
- `Raycast(origin, direction, distance, &hit)`
- **Global Settings**:
    - `Physics::Global::SetGravity(vec3)`
    - `Physics::Global::SetAirResistance(float)`

### Math Utilities (`C3D::Math`) [NEW]
Standard game development math helpers.

- `RandomRange(min, max)` / `RandomInt(min, max)`
- `Lerp(a, b, t)` (Linear Interpolation)
- `Clamp(val, min, max)`
- `SmoothDamp(curr, target, &vel, smoothTime, dt)` (Cinema-smooth movement)

---

## Media & Visuals

### Graphics (`C3D::Graphics`)
Modify materials and post-processing effects.

- `SetAlbedo(obj, color)`
- `SetMetallic(obj, value)`
- `SetRoughness(obj, value)`
- **Post-Processing**:
    - `SetSunBloom(intensity)`
    - `SetMoonBloom(intensity)`
    - `SetSSR(bool)` (Screen Space Reflections)

### Environment (`C3D::Environment`) [EXPANDED]
Refine atmospheric and world settings.

- `SetTimeOfDay(hour)` (0.0 - 24.0)
- `SetSkyboxEnabled(bool)`
- `SetCloudsEnabled(bool)`
- `SetCloudDensity(float)` / `SetCloudHeight(float)`
- `SetFogDensity(float)` / `SetFogColor(color)`

---

## UI & Scene

### UI Creation (`C3D::UI`)
Build dynamic menus and HUDs on the fly.

```cpp
C3D::UI::CreateButton("StartBtn", "Start Game", {100, 100}, {200, 50}, [](){
    C3D::Log::Info("Game Starting!");
});
```

- `CreateButton(name, text, pos, size, callback)`
- `CreateText(name, content, pos, color)`
- `CreateSlider(name, label, &value, min, max, pos, size)`
- `ClearElements()`

### Callbacks
Control interaction for any button or UI element by its unique name.

- `SetOnClick(name, callback)`: Standard click event.
- `SetOnHold(name, callback)`: Triggers every frame while the element is held.
- `SetOnRelease(name, callback)`: Triggers when the element is released.
- `SetOnHoverEnter(name, callback)`: Triggers once when the cursor enters.
- `SetOnHoverExit(name, callback)`: Triggers once when the cursor leaves.

### Scene & Hierarchy (`C3D::Scene`) [EXPANDED]
Manage levels and parent-child relationships.

- `Load("level_path")`
- `FindWithTag("Enemy")`
- `FindManyWithTag("Pickup")`
- **Hierarchy Tools**:
    - `GetParent(obj)`
    - `GetChildren(obj)` -> Returns vector of child pointers
    - `GetRootObjects()` -> Returns all top-level objects
