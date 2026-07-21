# NanoRender OpenGL

NanoRender OpenGL is a renderer developed incrementally to move a complete
graphics pipeline from CPU-side pixel processing to OpenGL and GLSL. Each
milestone keeps the application buildable, adds one focused GPU capability,
and records its validation and visual evidence here.

## Current Status

### OpenGL Foundation

The foundation creates an OpenGL 4.1 Core context with GLFW and GLAD and keeps a
resizable render loop. GLFW, GLM, and MicroUI are pinned through CMake.

```powershell
.\build\Release\nanorender_opengl.exe --validate foundation
```

Result: `Foundation validation passed.`

### HW2 Task 1 - Loading and Inspecting 3D Data

`src/mesh.cpp` loads OBJ positions and faces into GLM vertices and zero-based
triangle indices. The test tetrahedron has 4 vertices and 4 faces; the counts
are shown in the window title and console. Polygon faces, slash-separated face
tokens, and negative indices are supported.

```powershell
.\build\Release\nanorender_opengl.exe --validate hw2-task1
```

Result: `vertices=4 faces=4 valid_indices=yes`.

### HW2 Task 2 - Normalization and Viewport Fitting

The mesh bounds provide its center and largest dimension. One uniform scale
maps that dimension to 40% of the smaller framebuffer dimension, and a
translation centers it. The fit is recalculated after framebuffer resizing.

```powershell
.\build\Release\nanorender_opengl.exe --validate hw2-task2
```

Result at 1280 x 720: bounds `(-1,-1,-1)` to `(1,1,1)`, scale `144`,
translation `(640,360,0)`, and all fitted vertices inside the viewport.
Rendering begins in Task 3.

## Build and Run

Requirements:

- CMake 3.20 or newer
- A C++20 compiler
- Git
- A graphics driver supporting OpenGL 4.1 or newer

Configure and build:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Run on Windows:

```powershell
.\build\Release\nanorender_opengl.exe
```

## Roadmap

This roadmap follows the engine-related task order in the original HW2-HW5
project. A task is checked only after its implementation, validation,
documentation, focused commit, and push are complete.

### Foundation

- [x] Establish the OpenGL 4.1 Core foundation with GLFW and GLAD

### HW2 - Wireframe Viewer and Geometric Transformations

- [x] Task 0: Integrate GLM (included in the OpenGL foundation)
- [x] Task 1: Load OBJ data and display mesh information
- [x] Task 2: Calculate mesh bounds, normalization, and viewport fitting
- [ ] Task 3: Render an indexed orthographic wireframe mesh
- [ ] Task 4: Add separate local and world transformation controls
- [ ] Task 5: Apply model transformations in the vertex shader
- [ ] Task 6: Add direct keyboard or mouse transformation controls

### HW3 - Virtual Cameras and Projections

- [ ] Task 1: Render GPU debug axes and object bounding boxes
- [ ] Task 2: Add a virtual camera and view matrix
- [ ] Task 3: Add orthographic and perspective projection modes
- [ ] Task 4: Calculate, upload, transform, and visualize mesh normals

### HW4 - Triangle Rasterization and Depth Buffering

- [ ] Task 1: Add a GPU triangle bounding-box debug view
- [ ] Task 2: Add hardware triangle filling and visualize barycentric interpolation
- [ ] Task 3: Add GPU depth testing and depth-buffer visualization

### HW5 - Lighting, Materials, and Shading

- [ ] Task 1: Add light/material properties and ambient lighting
- [ ] Task 2: Add flat diffuse shading
- [ ] Task 3: Add specular lighting and reflection-vector debugging
- [ ] Task 4: Add per-fragment Phong shading

### Final Comparison

- [ ] Document the final CPU-to-GPU architecture, behavior, and performance comparison

Pair-programming extensions are outside this roadmap because they were not part
of the completed task reports used as the reference for this port.

## Project Structure

```text
nanorender-opengl/
|-- assets/         Direct visual evidence
|-- models/         Mesh files
|-- shaders/        GLSL shader programs
|-- src/            Renderer and application source
|-- third_party/    Generated GLAD loader
|-- CMakeLists.txt  Build and dependency configuration
`-- README.md       Documentation and progress log
```

## Development Workflow

Work is divided into small feature milestones. Every milestone must compile,
pass its validation mode, update this README, and receive a focused commit and
push before the next feature begins. Task notes stay brief and are appended in
implementation order from top to bottom. Visual milestones include a direct
screenshot or short GIF under `assets/`, whichever demonstrates the result more
clearly; animated evidence is kept below 3 MB.
