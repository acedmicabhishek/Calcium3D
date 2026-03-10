# Calcium3D: Auto-LOD Generation System

The Auto-LOD system in Calcium3D uses a sophisticated **Area-Weighted Quadric Error Metric (QEM)** algorithm to automatically generate four distinct Level of Detail (LOD) stages for any mesh.

---

## Execution Model: "Bake-Once" caching

To ensure maximum runtime performance, the calculation of LODs is **not** performed every frame. Instead, the engine follows a "Bake-on-Load" strategy:

1.  **Automatic Generation**: When a model is first imported or initialized in the project (i.e., when placing the model or loading the scene), the engine automatically triggers the QEM algorithm.
2.  **VRAM Caching**: The resulting 4 simplified meshes are "baked" and uploaded to **Video Memory (VRAM)** once.
3.  **Instant Swapping**: At runtime, the CPU simply calculates the distance to the camera and tells the GPU which pre-cached LOD stage to draw. This swap has **zero logic overhead**, saving CPU cycles.

---

## Core Algorithm: Area-Weighted QEM

The generation process involves three main phases:

### 1. Spatial Clustering
The mesh is subdivided into a 3D grid. The resolution of this grid depends on the target simplification ratio:
- **LOD 1 (Quality)**: Uses a high-density grid (96+ cells) for maximum fidelity.
- **LOD 2-4 (Aggressive)**: Grid resolution drops exponentially to force geometric collapse.

### 2. Quadric Error Calculation (Feature Preservation)
For every original triangle, we compute a **Quadric Matrix** representing the distance-to-plane error.
- **Area Weighting**: Larger triangles contribute more to the quadric, ensuring that major structural features (like wings or fuselages) dominate the simplification logic.
- **Error Accumulation**: Each clustering cell accumulates the quadric matrices of all triangles incident to its vertices.

### 3. Best-Candidate Selection (Edge & Texture Sharpness)
Instead of simply averaging vertex positions (which causes "melting" and UV shifting), our system uses a **Best-Candidate Search**:
- We solve the quadric system to find the mathematical **Optimal Position** that minimizes distance to all incident planes.
- We then compare this optimal point against all **Original Vertices** within that cell.
- The vertex that yields the lowest quadric error is selected. This preserves sharp edges, corners, and original UV coordinates, preventing texture "warping."

---

## Dynamic LOD Stages

The engine generates 4 levels with the following default geometric ratios:

| Level | Ratio | Description |
| :--- | :--- | :--- |
| **LOD 1** | 50.0% | **Quality Stage**: Visually indistinguishable from original, UV-perfect. |
| **LOD 2** | 5.0% | **Compressed Stage**: Aggressive reduction of fine details while keeping shape. |
| **LOD 3** | 1.0% | **Extreme Stage**: Simplified proxy for mid-to-long distance. |
| **LOD 4** | 0.2% | **Borderline Stage**: Minimal triangles, for ultra-long distance performance. |

