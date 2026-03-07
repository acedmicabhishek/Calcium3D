### Case Study: Envyobj

After a systematic isolation test, the TRUE root cause was found in `GeometryPass.cpp`.

**The Bug:**
When `enablePointShadows` was set to `false` in the build project.json, the geometry pass skipped binding point shadow cubemap textures to units 5-8. The fragment shader's `samplerCube` uniforms, left uninitialized, defaulted to **texture unit 0**.

**The Conflict:**
`tex0` (the diffuse texture) is also bound to unit 0. When the driver saw a `samplerCube` and a `sampler2D` sharing unit 0, it caused a conflict that rendered all geometry invisible.

**The Fix:**
1.  **GeometryPass.cpp:** Always bind the cubemaps to units 5-8 regardless of whether shadows are enabled. This ensures no sampler defaults to unit 0.
2.  **Build Export:** Separated environment, graphics (SSR/MSAA), and camera settings into proper top-level JSON keys in `project.json`.

