# Calcium3D Material & Texture System

## Overview

Every `GameObject` in Calcium3D has a `Material` that controls how it appears when lit. The material system uses **Blinn-Phong** shading with per-object properties, allowing each object to have a unique look.

---

## Material Properties

| Property | Type | Default | Range | Description |
|---|---|---|---|---|
| `albedo` | `vec3` | (0.8, 0.8, 0.8) | 0–1 per channel | Base color of the surface |
| `metallic` | `float` | 0.0 | 0–1 | 0 = plastic/dielectric, 1 = metallic sheen |
| `roughness` | `float` | 0.5 | 0–1 | 0 = mirror-smooth, 1 = completely matte |
| `ao` | `float` | 1.0 | 0–1 | Ambient occlusion — darkens crevices |
| `shininess` | `float` | 32.0 | 1–256 | Blinn-Phong specular exponent |
| `useTexture` | `bool` | true | — | When false, ignores textures and uses pure albedo |
| `diffuseTexture` | `string` | "" | — | Path to diffuse texture (future use) |
| `specularTexture` | `string` | "" | — | Path to specular map (future use) |

---

## How Rendering Works

### Pipeline Flow

```
GameObject → Renderer::RenderScene() → Shader Uniforms → default.frag → Pixel Color
```

1. **Renderer** iterates over every `GameObject` in the scene
2. For each object, it sets the material uniforms on the GPU:
   ```cpp
   shader.setVec3("material.albedo", obj.material.albedo);
   shader.setFloat("material.metallic", obj.material.metallic);
   shader.setFloat("material.roughness", obj.material.roughness);
   shader.setFloat("material.ao", obj.material.ao);
   shader.setFloat("material.shininess", obj.material.shininess);
   shader.setBool("material.useTexture", obj.material.useTexture);
   ```
3. The object's mesh is drawn with those uniforms active

### Shader Logic (`default.frag`)

The fragment shader calculates final color in three lighting passes:

#### 1. Ambient
```glsl
totalLighting += ambientStrength * baseColor * material.ao;
```
- Base illumination so nothing is fully black
- `ao` darkens areas that should receive less indirect light

#### 2. Directional Lights (Sun & Moon)
```glsl
// Diffuse: how directly the surface faces the light
float diff = max(dot(normal, lightDir), 0.0);

// Specular: Blinn-Phong halfway vector
vec3 halfwayDir = normalize(lightDir + viewDir);
float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
```
- **Diffuse** is tinted by `albedo`
- **Specular** is controlled by `roughness` (lower = brighter highlights)
- **Metallic** tints specular toward the base color (metals reflect their own color)

#### 3. Point Lights
Same Blinn-Phong calculation with distance attenuation:
```glsl
float attenuation = 1.0 / (constant + linear * dist + quadratic * dist²);
```

---

## Texture System

### How Textures Interact with Materials

```
Final Color = texture(tex0) * material.albedo    (when useTexture = true)
Final Color = material.albedo                     (when useTexture = false)
```

- **`tex0`** (slot 0) = Diffuse texture — multiplied by `albedo` to tint it
- **`tex1`** (slot 1) = Specular map — controls per-pixel specular intensity

When `useTexture` is **false**, the object renders as a solid color using only `albedo`. This is useful for prototyping or stylized art.

### Texture Loading

Textures are loaded through `ResourceManager`:
```cpp
ResourceManager::LoadTexture("name", "path/to/texture.png", "diffuse", 0);
```

In standalone builds, paths starting with `../Resource/` are automatically remapped to `Internal/Resource/` by `ResourceManager::ResolvePath()`.

### Default Textures

The engine loads two default textures at startup:
- `defaultDiffuse` — `../Resource/default/texture/DefaultTex.png` (slot 0)
- `defaultSpecular` — same texture used as specular map (slot 1)

These are used by any mesh that doesn't specify its own textures.

---

## Serialization

Materials are saved/loaded with scenes via JSON:

```json
{
  "material": {
    "albedo": [0.8, 0.2, 0.1],
    "metallic": 0.5,
    "roughness": 0.3,
    "ao": 1.0,
    "shininess": 64.0,
    "useTexture": false,
    "diffuseTexture": "",
    "specularTexture": ""
  }
}
```

---

## Editor Inspector

When you select an object, the Inspector shows the Material section with:
- **Color picker** for Albedo
- **Sliders** for Metallic, Roughness, and AO (0–1)
- **Drag input** for Shininess (1–256)
- **Checkbox** for Use Texture toggle

---

## Examples

| Look | Albedo | Metallic | Roughness | Shininess |
|---|---|---|---|---|
| Shiny plastic | (1, 0, 0) | 0.0 | 0.1 | 128 |
| Matte clay | (0.8, 0.6, 0.4) | 0.0 | 0.9 | 8 |
| Polished gold | (1, 0.8, 0.2) | 1.0 | 0.1 | 256 |
| Brushed steel | (0.7, 0.7, 0.7) | 1.0 | 0.5 | 64 |
| Rubber | (0.1, 0.1, 0.1) | 0.0 | 1.0 | 4 |

---

## File Locations

| File | Purpose |
|---|---|
| `src/Renderer/Material.h` | Material struct definition + serialization |
| `src/Scene/Scene.h` | `GameObject` contains `Material material` |
| `shaders/passes/geometry/default.frag` | Fragment shader with material uniforms |
| `src/Renderer/Renderer.cpp` | Sets per-object material uniforms |
| `src/Scene/SceneIO.cpp` | Saves/loads material with scenes |
| `src/Editor/EditorLayer.cpp` | Inspector UI for material editing |
