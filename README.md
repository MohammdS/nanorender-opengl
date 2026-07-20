# NanoRender OpenGL

NanoRender OpenGL is a renderer developed incrementally to move a complete
graphics pipeline from CPU-side pixel processing to OpenGL and GLSL. Each
milestone keeps the application buildable, adds one focused GPU capability,
and records its validation and visual evidence here.

## Current Status

### OpenGL Foundation

The project currently creates an OpenGL 4.1 Core context with GLFW, loads the
required OpenGL functions through GLAD, and maintains a resizable render loop.
The framebuffer is cleared to a fixed dark color, and pressing `Escape` closes
the application.

GLFW, GLM, and MicroUI are pinned through CMake. GLAD is generated for only the
OpenGL 4.1 Core API and stored under `third_party/glad`, so normal builds do not
require Python or a generator package.

No geometry is rendered in this milestone. GPU line rasterization is the next
feature.

### Validation

The hidden foundation validation creates a real OpenGL context, clears one
frame, reads the center pixel back, and fails if the framebuffer color or
OpenGL state is incorrect:

```powershell
.\build\Release\nanorender_opengl.exe --validate foundation
```

Verified Release output:

```text
OpenGL 4.1 initialized.
Foundation framebuffer: 1280x720, center rgb(14, 19, 28)
Foundation validation passed.
```

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

This roadmap follows the task order in the original HW1-HW5 project. A task is
checked only after its implementation, validation, documentation, focused
commit, and push are complete.

### Foundation

- [x] Establish the OpenGL 4.1 Core foundation with GLFW and GLAD

### HW1 - Basic Graphics and Immediate Mode GUI

- [ ] Task 1: Render the animated plasma background with a fullscreen GPU shader
- [ ] Task 2: Initialize MicroUI and display a simple interactive widget
- [ ] Task 3: Connect GLFW input callbacks and add a keyboard visual effect
- [ ] Task 4: Render MicroUI through a GPU bridge and demonstrate a visual UI transform
- [ ] Task 5: Bind MicroUI controls to the GPU background shader state
- [ ] Task 6: Add GPU line rasterization and the interactive line-drawing canvas

### HW2 - Wireframe Viewer and Geometric Transformations

- [x] Task 0: Integrate GLM (included in the OpenGL foundation)
- [ ] Task 1: Load OBJ data and display mesh information
- [ ] Task 2: Calculate mesh bounds, normalization, and viewport fitting
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
push before the next feature begins. Visual milestones include a direct
screenshot or short GIF under `assets/`, whichever demonstrates the result
more clearly; animated evidence is kept below 3 MB.
