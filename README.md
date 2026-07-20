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

- [x] Establish the OpenGL foundation
- [ ] Add GPU line rasterization
- [ ] Add the GPU MicroUI bridge and interactive drawing
- [ ] Load and render indexed wireframe meshes
- [ ] Move object transformations to the vertex shader
- [ ] Add debug axes and bounding boxes
- [ ] Add the virtual camera
- [ ] Add orthographic and perspective projection modes
- [ ] Upload and visualize normals
- [ ] Add hardware triangle rasterization
- [ ] Add GPU depth testing and depth visualization
- [ ] Add materials and ambient lighting
- [ ] Add flat diffuse shading
- [ ] Add specular lighting
- [ ] Add per-fragment Phong shading
- [ ] Document the final CPU-to-GPU comparison

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
push before the next feature begins. Visual milestones also include a direct
GIF under `assets/`, kept below 3 MB.
