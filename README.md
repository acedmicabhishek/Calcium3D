![C3D Preview](C3dpeview.png)

# Under Development

# Calcium3D - A Lightweight 3D Game Engine

Calcium3D is a lightweight 3D game engine built with C++ and OpenGL. It features an in-engine editor for scene manipulation, various rendering capabilities, and a modular architecture.

## Features

*   **Real-time 3D Rendering:**
    *   Basic object primitives (cubes, planes, spheres).
    *   Support for loading complex 3D models (.obj, .gltf).
    *   Texture mapping.
    *   Multiple lighting sources:
        *   Local point lights.
        *   Configurable global directional light (sun) with adjustable color, intensity, and position presets.
    *   Dynamic skybox rendering.
    *   Cel-shading (toon shading) effect.
    *   Post-processing effects via framebuffer.
    *   Multi-Sample Anti-Aliasing (MSAA) for smoother visuals.

*   **In-Engine Editor (ImGui-based):**
    *   Intuitive user interface for scene management.
    *   Object selection and manipulation using translate, rotate, and scale gizmos.
    *   Real-time adjustment of camera properties (FOV, near/far planes, speed, sensitivity).
    *   Performance monitoring (FPS, frame time, delta time).
    *   Scene object creation (cubes, planes, spheres) and deletion.
    *   Clear selection and destroy all objects functionalities.
    *   Toggle between Edit and Play modes.
    *   Integrated logger for engine messages.

*   **Camera System:**
    *   First-person camera with free-look and movement.
    *   Adjustable mouse sensitivity and movement speed.

## Technologies Used

*   **C++:** Core engine logic and development.
*   **OpenGL:** Graphics rendering API.
*   **GLFW:** Windowing and input management.
*   **GLAD:** OpenGL function loader.
*   **GLM (OpenGL Mathematics):** Vector and matrix mathematics library.
*   **ImGui:** Immediate Mode Graphical User Interface for the in-engine editor.
*   **stb_image:** Single-file library for loading various image formats.
*   **tiny_obj_loader:** Library for loading .obj 3D models.
*   **nlohmann/json:** JSON library for parsing (likely for .gltf models).

## Getting Started

### Prerequisites

*   C++ Compiler (e.g., g++)
*   CMake (for building the project)
*   OpenGL compatible graphics card and drivers

### Building and Running

1.  **Clone the repository:**
    ```bash
    git clone [repository_url_here]
    cd Calcium3D
    ```
2.  **Build the project:**
    **Auto build**
    ```bash
    ./compile.sh
    ```
    **with Cmake**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```
3.  **Run the engine:**
    ```bash
    ./Calcium3D (executable will be generated inside the build dir)
    or 
    ./run.sh
    ```

## Contributing

 fork and make chnages and pull request
 that is it , ill check if your changes are worth or not.

## License

Do whatever the fuck you want (GPL)